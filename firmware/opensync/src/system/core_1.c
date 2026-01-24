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

PIO pio_clocks = pio0;
PIO pio_output = pio1;

const uint32_t ARM_SEQUENCER = 1;

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

    sequencer_clocks_init(
        sequencer_clock_config,
        pio_clocks
    );

    sequencer_output_init(
        sequencer_pulse_config,
        pio_output
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
            "Internal Message: Starting to configure clock state machines\r\n"
        );

        // Configure all active channels
        sequencer_clock_sm_config_active();

        debug_message_print(
            debug_status_local,
            "Internal Message: Starting to configure output state machines\r\n"
        );

        sequencer_output_sm_config_active();

        // Start the state machines
        if (sequencer_status_get() != ABORT_REQUESTED)
        {
            sequencer_status_set(RUNNING);

            // Get masks for all active state mahcines
            uint pio_clocks_sm_mask = sequencer_clock_sm_mask_get();
            uint pio_output_sm_mask = sequencer_output_sm_mask_get();

            debug_message_print(
                debug_status_local,
                "Internal Message: Starting all active state machines\r\n"
            );

            pio_enable_sm_multi_mask_in_sync(
                pio_clocks,
                0u,
                pio_clocks_sm_mask,
                pio_output_sm_mask
            );

//            debug_message_print(
//                debug_status_local,
//                "Internal Message: Starting outputs state machines\r\n"
//            );

//            pio_enable_sm_mask_in_sync(
//                pio_output,
//                pio_output_sm_mask
//            );

//            debug_message_print(
//                debug_status_local,
//                "Internal Message: Starting clocks state machines\r\n"
//            );

//            pio_enable_sm_mask_in_sync(
//                pio_clocks,
//                pio_clocks_sm_mask
//            );

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
        
        if (sequencer_status_get() != ABORTING)
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

struct clock_config* sequencer_clock_config_get()
{
    return sequencer_clock_config;
}

struct pulse_config* sequencer_pulse_config_get()
{
    return sequencer_pulse_config;
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

            sequencer_clock_sm_config(
                &sequencer_clock_config[i],
                offset_clock_freerun
            );

            debug_message_print_i(
                debug_status_local_func,
                "Internal Message: Finished configuring clock state machine %i\r\n",
                i
            );
        }
    }
}


// NOTE: Has debug messages incl.
void sequencer_output_sm_config_active()
{
    uint32_t debug_status_local_func = debug_status_get();

    if (!sequencer_pulse_conflict_check())
    {
        debug_message_print(
            debug_status_local_func,
            "Internal Message: Invalid pulse sequencer configuration (channel conflicts)\r\n"
        );

        sequencer_status_set(ABORT_REQUESTED);
    }

    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        if (sequencer_pulse_config[i].active == true)
        {
            if(!sequencer_pulse_validate(&sequencer_pulse_config[i]))
            {
                debug_message_print_i(
                    debug_status_local_func,
                    "Internal Message: Invalid pulse sequencer configuration for channel %i\r\n",
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
                offset_output
            );

            debug_message_print_i(
                debug_status_local_func,
                "Internal Message: Finished configuring output state machine for channel %i\r\n",
                i
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
            (sequencer_status_get() != ABORT_REQUESTED))
            {
                sleep_us(100);
            }
    }
}


void sequencer_sm_active_free()
{
    // Cleanup clock configs
    for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
    {
        if (sequencer_clock_config[i].configured == true)
        {
            sequencer_clock_free(
                &sequencer_clock_config[i]
            );
        }
    }
        
    // Cleanup output configs
    for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
    {
        if (sequencer_pulse_config[i].configured == true)
        {
            sequencer_output_free(
                &sequencer_pulse_config[i]
            );
        }
    }
}


bool sequencer_pulse_conflict_check()
{
    #define NUM_BITS 32
    const uint32_t MAX_BITS = 1;

    uint32_t channel_state[CLOCKS_MAX][NUM_BITS] = {0};

    // Accumulate all the bit information into a 2D array
    for (uint32_t chan_id = 0; chan_id < CLOCKS_MAX; chan_id++)
    {
        for (uint32_t i = 0; i < PULSE_INSTRUCTIONS_MAX; i++)
        {
            uint32_t state = sequencer_pulse_config[chan_id].instructions[i];

            // The binary array is backwards, but this doesn't matter since we are only looking for nums > 1
            uint32_t j = 0;
            while (state > 0)
            {
                channel_state[chan_id][j] = state % 2;
                state /= 2;

                j++;
            }
        }
    }

    // Check the array sums for values > 1 which indicates that there is a channel conflict
    for (uint32_t i = 0; i < NUM_BITS; i++)
    {
        uint32_t channel_sum = 0;

        for (uint32_t chan_id = 0; chan_id < CLOCKS_MAX; chan_id++)
        {
            channel_sum += channel_state[chan_id][i];
        }

        if (channel_sum > MAX_BITS)
        {
            return 0;
        }
    }

    return 1;
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


bool sequencer_clock_type_set(
    uint32_t clock_id,
    uint32_t requested_type
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Clock types must be zero to max enum type (CLOCK_TRIGGERED in this case)
    if (requested_type > CLOCK_TRIGGERED)
    {
        return 0;
    }

    sequencer_clock_config[clock_id].clock_type = requested_type;

    return 1;
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

    sequencer_clock_config[clock_id].clock_divider = (uint) clock_divider_copy;

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

    sequencer_pulse_config[pulse_id].clock_divider = (uint) clock_divider_copy;

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

    // Validate trigger ID (can't be larger than or equal to max triggers)
    // TODO: Make this its own function
    if(trigger_pin_id >= TRIGGERS_MAX)
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


bool clock_unit_offset_set(
    uint32_t clock_id,
    double units_offset
) {
    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Make sure the offset is never 0
    if(units_offset < SEQUENCER_DOUBLE_EPS)
    {
        return 0;
    }

    sequencer_clock_config[clock_id].unit_offset = units_offset;

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


bool pulse_unit_offset_set(
    uint32_t pulse_id,
    double units_offset
) {
    const double eps = 0.000001;

    // Validate clock ID
    if(!clock_id_validate(pulse_id))
    {
        return 0;
    }

    // Make sure the offset is never 0
    if(units_offset < eps)
    {
        return 0;
    }

    sequencer_pulse_config[pulse_id].unit_offset = units_offset;

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