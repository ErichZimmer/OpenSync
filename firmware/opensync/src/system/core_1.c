#include "core_1.h"
#include "fast_serial.h"


static struct clock_config sequencer_clock_config[CLOCKS_MAX];
static struct pulse_config sequencer_pulse_config[CLOCKS_MAX];

static uint clock_divider = 60000;

PIO pio_clocks = pio0;
PIO pio_output = pio1;

uint reps = 50;

const uint32_t ARM_SEQUENCER = 1;


void core_1_init()
{
    // Freerun PIO clock program
    uint offset_clock_freerun = sequencer_program_freerun_add(
        pio_clocks
    );

    // Triggered PIO clock program
    uint offset_clock_triggered = sequencer_program_triggered_add(
        pio_clocks
    );

    // Sequencer PIO pulse program
    uint offset_output = sequencer_program_output_add(
        pio_output
    );

    // Make an array of ones to insert a dummy program 
    uint32_t dummy_clock_instruction[CLOCK_INSTRUCTIONS_MAX] = {0};

    // Add some random instructions to output
    /*
    Important Note:
    The clock output instruction are broken up into two parts: the pulse
    frequency and the amount of repitions to perform. For instance, every even 
    instruction starting from zero (e.g., 0, 2, 4, 6, 8...) sets the pulse
    repition amount. The repitition frequency is set at every odd instruction
    index (e.g., 1, 3, 5, 7, 9...).

    A repition instruction of zero skips the instruction pair, so zeros are okay here.
    */


    // Create some clock pattern
    dummy_clock_instruction[0] = reps; // The amount of reps to perform
    dummy_clock_instruction[1] = 10; // the repition rate (must be higher than the total duration of the pulse sequencer)

    // Make an array of ones to insert a dummy program
    uint32_t dummy_output_instruction[PULSE_INSTRUCTIONS_MAX] = {0};

    // Add some random instructions to output
    /*
    Important Note:
    The pulse output instruction are broken up into two parts: the output
    state and the delay duration to the next instruction. For instance,
    every even instruction starting from zero (e.g., 0, 2, 4, 6, 8...) sets
    the output state. Only the first 12 bits are used to set the output state
    while the rest are discarded. The delay instructions are at every odd
    instruction index (e.g., 1, 3, 5, 7, 9...) and are used in a delay loop.
    The final two instruction at the end of the instruction buffer are used
    to set the output state to idle and a terminating flag.
    
    // DO NOT FORGET TO SET THE TERMINATING FLAGS (0) AND MAKE SURE ALL OTHER INSTRUCTIONS ARE NON-ZERO!!!!
    */

    // Set all delay instructions to 1 (they can't be zero)
    for (int i = 1; i < PULSE_INSTRUCTIONS_MAX; i += 2)
    {
        dummy_output_instruction[i] = 1; // 1 cycle delay
    }

    // Create some pulse pattern
    dummy_output_instruction[0] = 2048; // Channel 12 square wave
    dummy_output_instruction[1] = 4000000;
                                  
    dummy_output_instruction[2] = 1024; // Channel 11 square wave
    dummy_output_instruction[3] = 4000000;

    dummy_output_instruction[4] = 512; // Channel 10 square wave
    dummy_output_instruction[5] = 4000000;

    dummy_output_instruction[6] = 2560; // Channel 10 and 12
    dummy_output_instruction[7] = 4000000;


    // Make sure the final instructions are zero
    dummy_output_instruction[PULSE_INSTRUCTIONS_MAX - 1] = 0;
    dummy_output_instruction[PULSE_INSTRUCTIONS_MAX - 2] = 0;

    // INTERNAL CLOCK //
    sequencer_clocks_init(
        sequencer_clock_config,
        pio_clocks
    );

    // Add instruction to ouput channel 0
    sequencer_clock_insert_instructions_internal(
        &sequencer_clock_config[0],
        dummy_clock_instruction
    );

    sequencer_clock_configure(
        &sequencer_clock_config[0],
        INTERNAL_CLOCK_PINS[0],
        EXTERNAL_TRIGGER_PINS[0],
        0
    );

    // OUTPUT //
    sequencer_output_init(
        sequencer_pulse_config,
        pio_output
    );

    // Add instruction to ouput channel 0
    sequencer_output_insert_instructions(
        &sequencer_pulse_config[0],
        dummy_output_instruction
    );

    // Set clock ID to 0
    sequencer_output_configure(
        &sequencer_pulse_config[0],
        INTERNAL_CLOCK_PINS[0]
    );


    multicore_fifo_push_blocking(0);

    while(true)
    {
        uint32_t arming_status = multicore_fifo_pop_blocking();

        if (arming_status != ARM_SEQUENCER)
        {
            continue;
        }

        uint32_t debug_status_local = debug_status_get();
        
        sequencer_status_set(ARMING);

        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
            fast_serial_printf("Internal Message: Clock divider: %i\r\n", clock_divider);

            // Print clock and pulse configs
            serial_print_clock_configs(sequencer_clock_config);
            serial_print_pulse_configs(sequencer_pulse_config);
        }

        if (debug_status_local == SEQUENCER_DEBUG)
        {
            fast_serial_printf("Internal Message: Aborting arming sequence due to debugging level 1\r\n");

            sequencer_status_set(ABORTED);

            // Break current arming sequence and abort
            continue;
        }


        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
            fast_serial_printf("Internal Message: Starting to configure state machines\r\n");
        }

        sequencer_clock_freerun_sm_config(
            &sequencer_clock_config[0],
            offset_clock_freerun,
            clock_divider
        );

//        sequencer_output_sm_config(
//            &sequencer_pulse_config[0],
//            offset_output,
//            clock_divider,
//            reps
//        );

        uint pio_clocks_sm_mask = 0;
        uint pio_output_sm_mask = 0;

        for (uint32_t i = 0; i < CLOCKS_MAX; i++)
        {
            if (sequencer_clock_config[i].active == true)
            {
                pio_clocks_sm_mask |= 1u << i;
            }
        }

        for (uint32_t i = 0; i < CLOCKS_MAX; i++)
        {
            if (sequencer_pulse_config[i].active == true)
            {
                pio_output_sm_mask |= 1u << i;
            }
        }

        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
            fast_serial_printf(
                "DMA clock id %i status: %i\r\n", 
                0, dma_channel_is_busy(sequencer_clock_config[0].dma_chan)
            );       
        }

        sequencer_status_set(RUNNING);

//        pio_enable_sm_multi_mask_in_sync(
//            pio_clocks,
//            0u,
//            pio_clocks_sm_mask,
//            pio_output_sm_mask
//        );

//        if (debug_status_local == SEQUENCER_DEBUG)
//        {
//            fast_serial_printf("Internal Message: Starting outputs state machine\r\n");
//        }

//        pio_enable_sm_mask_in_sync(
//            pio_output,
//            pio_output_sm_mask
//        );

        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
            fast_serial_printf("Internal Message: Starting clocks state machine\r\n");
        }

        pio_enable_sm_mask_in_sync(
            pio_clocks,
            pio_clocks_sm_mask
        );

        // Stall core until all processes are done
        for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
        {
            if (debug_status_local != SEQUENCER_DNDEBUG)
            {  
            fast_serial_printf("Internal Message: Entering stall for clock id: %i\r\n", i);
            }

            if (sequencer_clock_config[i].active != true)
            {
                continue;
            }

            while(dma_channel_is_busy(sequencer_clock_config[i].dma_chan)){ }
        }

        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
            fast_serial_printf("Internal Message: Cleaning up state machines\r\n");
        }

        sequencer_status_set(DISARMING);

        // Cleanup clock configs
        for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
        {
            if (sequencer_clock_config[i].active == true)
            {
                sequencer_clock_sm_free(
                    &sequencer_clock_config[i]
                );
            }
        }
            

        // Cleanup output configs
//        for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
//        {
//            if (sequencer_pulse_config[i].active == true)
//            {
//                sequencer_output_sm_free(
//                    &sequencer_pulse_config[i]
//                );
//            }
//     }

        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
            fast_serial_printf("Internal Message: Sequencer reset to IDLE status\r\n");
        }
        
        sequencer_status_set(IDLE);
    }
}


bool clock_divider_set(
    uint32_t clock_divider_copy
) {
    // Make sure the clock divider is supported
    if ((clock_divider_copy > CLOCK_DIVIDER_MAX) ||
        (clock_divider_copy == 0))
    {
        return 0;
    }

    clock_divider = clock_divider_copy;

    return 1;
}


// For now, clock IDs and trigger IDs have the same range (e.g., [0..2])
bool clock_id_validate(
    uint32_t clock_id
) {
    // Make sure the internal clock ID is supported
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        uint32_t supported_clock_id = INTERNAL_CLOCK_IDS[i];

        if (clock_id == supported_clock_id)
        {
            return 1;
        }
    }

    return 0;
}


bool clock_pin_trigger_set(
    uint32_t clock_id,
    uint32_t trigger_pin_id
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Validate trigger ID (same as clock ID)
    if(!clock_id_validate(trigger_pin_id))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].trigger_pin = EXTERNAL_TRIGGER_PINS[trigger_pin_id];

    return 1;
}


bool clock_reps_trigger_set(
    uint32_t clock_id,
    uint32_t trigger_reps
) {
    // Validate clock ID
    if (!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Validate repitition number
    if ((trigger_reps > ITERATIONS_MAX) ||
        (trigger_reps == 0))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].trigger_reps = trigger_reps;

    return 1;
}


bool clock_trigger_instructions_load(
    uint32_t clock_id,
    uint32_t trigger_skips,
    uint32_t trigger_delay
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Validate trigger skips
    if ((trigger_skips > TRIGGER_SKIPS_MAX) ||
        (trigger_skips == 0))
    {
        return 0;
    }

    // Validate trigger delay
    if (trigger_delay == 0)
    {
        return 0;
    }

    // Create trigger instructions buffer
    uint32_t instructions[CLOCK_TRIGGERS_MAX] = {0};

    instructions[0] = trigger_skips;
    instructions[1] = trigger_delay;

    sequencer_clock_insert_instructions_triggered(
        &sequencer_clock_config[clock_id],
        instructions
    );

    return 1;
}


bool clock_instructions_load(
    uint32_t clock_id,
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX]
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    sequencer_clock_insert_instructions_internal(
        &sequencer_clock_config[clock_id],
        instructions
    );

    return 1;
}


bool pulse_pin_clock_set(
    uint32_t pulse_id,
    uint32_t clock_id
) {

    // Validate clock ID
    if(!clock_id_validate(pulse_id))
    {
        return 0;
    }

    // Validate trigger ID (same as clock ID)
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    sequencer_pulse_config[pulse_id].clock_pin = INTERNAL_CLOCK_PINS[clock_id];

    return 1;
}

bool pulse_instructions_load(
    uint32_t pulse_id,
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX]
) {

    // Validate pulse ID (same as clock ID)
    if(!clock_id_validate(pulse_id))
    {
        return 0;
    }

    sequencer_output_insert_instructions(
        &sequencer_pulse_config[pulse_id],
        instructions
    );

    return 1;
}