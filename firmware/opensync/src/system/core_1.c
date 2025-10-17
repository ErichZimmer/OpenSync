#include "core_1.h"

#include <stdint.h>
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "fast_serial.h"

#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer/sequencer_common.h"
#include "sequencer/sequencer_clock.h"
#include "sequencer/sequencer_output.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "serial/serial_int_output.h"


static uint offset_clock_freerun = 0;
static uint offset_clock_triggered = 0;
static uint offset_output = 0;

static struct clock_config sequencer_clock_config[CLOCKS_MAX];
static struct pulse_config sequencer_pulse_config[CLOCKS_MAX];

const uint32_t CLOCK_FREERUN = 0;
const uint32_t CLOCK_TRIGGERED = 1;

static uint32_t clock_type = CLOCK_FREERUN;

PIO pio_clocks = pio0;
PIO pio_output = pio1;

const uint32_t ARM_SEQUENCER = 1;

uint clock_divider = 65200;
uint reps = 50;

void core_1_init()
{
    // Freerun PIO clock program
    offset_clock_freerun = sequencer_program_freerun_add(
        pio_clocks
    );

    // Triggered PIO clock program
    offset_clock_triggered = sequencer_program_triggered_add(
        pio_clocks
    );

    // Sequencer PIO pulse program
    offset_output = sequencer_program_output_add(
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
    dummy_clock_instruction[1] = 10; // the repition delay (controls rate) (must be higher than the total duration of the pulse sequencer)

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
    dummy_output_instruction[1] = 100;
                                  
    dummy_output_instruction[2] = 1024; // Channel 11 square wave
    dummy_output_instruction[3] = 100;

    dummy_output_instruction[4] = 512; // Channel 10 square wave
    dummy_output_instruction[5] = 100;

    dummy_output_instruction[6] = 2560; // Channel 10 and 12
    dummy_output_instruction[7] = 100;


    // Make sure the final instructions are zero
    dummy_output_instruction[PULSE_INSTRUCTIONS_MAX - 1] = 0;
    dummy_output_instruction[PULSE_INSTRUCTIONS_MAX - 2] = 0;

    // set clock channel to use
    uint32_t channel_to_use = 0;

    // INTERNAL CLOCK //
    sequencer_clocks_init(
        sequencer_clock_config,
        pio_clocks
    );

    // Add instruction to ouput channel 0
    sequencer_clock_insert_instructions_internal(
        &sequencer_clock_config[channel_to_use],
        dummy_clock_instruction
    );

    sequencer_clock_configure(
        &sequencer_clock_config[channel_to_use],
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
        &sequencer_pulse_config[channel_to_use],
        dummy_output_instruction
    );

    // Set clock ID to 0
    sequencer_output_configure(
        &sequencer_pulse_config[channel_to_use],
        INTERNAL_CLOCK_PINS[0]
    );

    // Enable clock and pulse channels to be used
    clock_sequencer_state_set(
        channel_to_use, // clock channel 0
        1 // enabled
    );

//    pulse_sequencer_state_set(
//        channel_to_use, // Output channel 0
//        1 // enabled
//    );

    // Set clock to freerun
    sequencer_clock_type_set(CLOCK_FREERUN);

    // Set clock dividers
    clock_divider_set(
        channel_to_use,
        clock_divider
    );

    pulse_divider_set(
        channel_to_use,
        clock_divider
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

        debug_message_print_i(
            debug_status_local,
            "Internal Message: Sequencer clock type (0=freerun; 1=triggered)=%i\r\n",
            clock_type
        );

        if (debug_status_local != SEQUENCER_DNDEBUG)
        {
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

        debug_message_print(
            debug_status_local,
            "Internal Message: Starting to configure state machines\r\n"
        );

        // Configure all active channels
        sequencer_clock_sm_config_active();
        sequencer_output_sm_config_active();

        // Start the state machines
        if (sequencer_status_get() != ABORT_REQUESTED)
        {
            sequencer_status_set(RUNNING);

            // Get masks for all active state mahcines
            uint pio_clocks_sm_mask = sequencer_clock_sm_mask_get();
            uint pio_output_sm_mask = sequencer_output_sm_mask_get();

    //        pio_enable_sm_multi_mask_in_sync(
    //            pio_clocks,
    //            0u,
    //            pio_clocks_sm_mask,
    //            pio_output_sm_mask
    //        );

            debug_message_print(
                debug_status_local,
                "Internal Message: Starting outputs state machines\r\n"
            );

            pio_enable_sm_mask_in_sync(
                pio_output,
                pio_output_sm_mask
            );

            debug_message_print(
                debug_status_local,
                "Internal Message: Starting clocks state machines\r\n"
            );

            pio_enable_sm_mask_in_sync(
                pio_clocks,
                pio_clocks_sm_mask
            );

            // Stall core until all processes are done
            sequencer_clock_sm_stall();
        }

        debug_message_print(
            debug_status_local,
            "Internal Message: Cleaning up state machines\r\n"
        );

        if (sequencer_status_get() != ABORT_REQUESTED)
        {
            sequencer_status_set(DISARMING);
        }
        
        else
        {
            sequencer_status_set(ABORTING);
        }

        // Cleanup state machines
        sequencer_sm_active_free();
        
        debug_message_print(
            debug_status_local,
            "Internal Message: Sequencer reset to IDLE status\r\n"
        );
        
        if (sequencer_status_get() != ABORT_REQUESTED)
        {
            sequencer_status_set(IDLE);
        }
        
        else
        {
            sequencer_status_set(ABORTED);
        }
    }
}


// NOTE: This is defined in core_1.c
void debug_message_print(
    uint32_t debug_status_local,
    char* message
) {
    if (debug_status_local != SEQUENCER_DNDEBUG)
    {
        fast_serial_printf(message);
    }
}


// NOTE: This is defined in core_1.c
void debug_message_print_i(
    uint32_t debug_status_local,
    char* message,
    int num
) {
    if (debug_status_local != SEQUENCER_DNDEBUG)
    {
        fast_serial_printf(message, num);
    }
}


// NOTE: Has debug messages incl.
void sequencer_clock_sm_config_active()
{
    uint32_t debug_status_local_func = debug_status_get();

    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        if (sequencer_clock_config[i].active == true)
        {
            debug_message_print_i(
                debug_status_local_func,
                "Internal Message: Starting to configure clock state machine %i\r\n",
                i
            );

            if (clock_type == CLOCK_FREERUN)
            {
                sequencer_clock_freerun_sm_config(
                    &sequencer_clock_config[i],
                    offset_clock_freerun
                );
            }
            else // Perhaps, make sure that the clock_type is correct?
            {
                sequencer_clock_triggered_sm_config(
                    &sequencer_clock_config[i],
                    offset_clock_freerun
                );
            }
        }
    }
}


// NOTE: Has debug messages incl.
void sequencer_output_sm_config_active()
{
    uint32_t debug_status_local_func = debug_status_get();

    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        if (sequencer_pulse_config[i].active == true)
        {
            if(!sequencer_pulse_validate(&sequencer_pulse_config[i]))
            {
                debug_message_print_i(
                    debug_status_local_func,
                    "Internal Message: Invalid pulse sequencer configuration for channel  %i\r\n",
                    i
                );

                sequencer_status_set(ABORT_REQUESTED);
                break;
            }

            debug_message_print_i(
                debug_status_local_func,
                "Internal Message: Starting to configure output state machine for channel %i\r\n",
                i
            );

            sequencer_output_sm_config(
                &sequencer_pulse_config[0],
                offset_output,
                reps
            );
        }
    }
}


uint sequencer_clock_sm_mask_get()
{
    uint pio_clocks_sm_mask = 0;
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        if (sequencer_clock_config[i].configured == true)
        {
            pio_clocks_sm_mask |= 1u << i;
        }
    }

    return pio_clocks_sm_mask;
}


uint sequencer_output_sm_mask_get()
{
    uint pio_output_sm_mask = 0;

    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        if (sequencer_pulse_config[i].configured == true)
        {
            pio_output_sm_mask |= 1u << i;
        }
    }

    return pio_output_sm_mask;
}


// NOTE: Has debug messages incl.
void sequencer_clock_sm_stall()
{
    uint32_t debug_status_local_func = debug_status_get();

    for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
    {
        debug_message_print_i(
            debug_status_local_func,
            "Internal Message: Entering stall for clock id: %i\r\n",
            i
        );

        if (sequencer_clock_config[i].configured != true)
        {
            continue;
        }

        while((dma_channel_is_busy(sequencer_clock_config[i].dma_chan)) &&
                (sequencer_status_get() != ABORT_REQUESTED)){ }
    }
}


void sequencer_sm_active_free()
{
    // Cleanup clock configs
    for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
    {
        if (sequencer_clock_config[i].configured == true)
        {
            sequencer_clock_sm_free(
                &sequencer_clock_config[i]
            );
        }
    }
        
    // Cleanup output configs
    for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
    {
        if (sequencer_pulse_config[i].configured == true)
        {
            sequencer_output_sm_free(
                &sequencer_pulse_config[i]
            );
        }
    }
}


bool sequencer_pulse_validate(
    struct pulse_config* config
) {
    const uint32_t FLAG_OFFSET = 2;
     const uint32_t TERM_FLAG = 0;

    // Check to see if all delay instructions are non-zero
    for (uint32_t i = 1; i < PULSE_INSTRUCTIONS_MAX - FLAG_OFFSET; i = i + 2)
    {
        if (config -> instructions[i] == TERM_FLAG)
        {
            return 0;
        }
    }

    // Validate terminator flags
    if ((config -> instructions[PULSE_INSTRUCTIONS_MAX - FLAG_OFFSET] != TERM_FLAG) ||
        (config -> instructions[PULSE_INSTRUCTIONS_MAX - FLAG_OFFSET + 1] != TERM_FLAG))
    {
        return 0;
    }

    return 1;
}


bool sequencer_clock_type_set(
    uint32_t requested_type
) {
    // Clock types can either be one or zero
    if (requested_type > 2)
    {
        return 0;
    }

    clock_type = requested_type;

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


bool clock_divider_set(
    uint32_t clock_id,
    uint32_t clock_divider_copy
) {
    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Make sure the clock divider is supported
    if ((clock_divider_copy > CLOCK_DIVIDER_MAX) ||
        (clock_divider_copy == 0))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].clock_divider = clock_divider_copy;

    return 1;
}


bool pulse_divider_set(
    uint32_t pulse_id,
    uint32_t clock_divider_copy
) {
    // Validate clock ID
    if(!clock_id_validate(pulse_id))
    {
        return 0;
    }

    // Make sure the clock divider is supported
    if ((clock_divider_copy > CLOCK_DIVIDER_MAX) ||
        (clock_divider_copy == 0))
    {
        return 0;
    }

    sequencer_pulse_config[pulse_id].clock_divider = clock_divider_copy;

    return 1;
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


bool clock_sequencer_state_set(
    uint32_t clock_id,
    bool clock_state
) {
    // Validate clock ID
    if (!clock_id_validate(clock_id))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].active = clock_state;

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
    if (trigger_skips > TRIGGER_SKIPS_MAX)
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


bool clock_sequencer_state_reset(
    uint32_t clock_id
) {
    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    sequencer_clock_config_reset(
        &sequencer_clock_config[clock_id]
    );

    return 1;
}


bool pulse_sequencer_state_set(
    uint32_t pulse_id,
    bool pulse_state
) {
    // Validate pulse ID (same as clock ID)
    if (!clock_id_validate(pulse_id))
    {
        return 0;
    }

    sequencer_pulse_config[pulse_id].active = pulse_state;

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

bool pulse_sequencer_state_reset(
    uint32_t pulse_id
) {
    // Validate pulse ID (same as clock ID)
    if(!clock_id_validate(pulse_id))
    {
        return 0;
    }

    sequencer_output_config_reset(
        &sequencer_pulse_config[pulse_id]
    );

    return 1;
}