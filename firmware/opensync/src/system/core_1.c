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

static struct clock_config sequencer_clock_config[CLOCKS_MAX];
static struct pulse_config sequencer_pulse_config[CLOCKS_MAX];

PIO pio_clocks = pio0;
PIO pio_output = pio1;

const uint32_t ARM_SEQUENCER = 1;

void core_1_init()
{
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
            "Internal Message: Starting to configure clock channel state machines\r\n"
        );

        // Configure all active channels
        sequencer_clock_sm_config_active();

        debug_message_print(
            debug_status_local,
            "Internal Message: Starting to configure pulse channel state machines\r\n"
        );

        sequencer_output_sm_config_active();

        // Start the state machines if everything configured properly
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


// return all three clock configs
struct clock_config* sequencer_clock_config_get()
{
    return sequencer_clock_config;
}


// return all three pulse configs
struct pulse_config* sequencer_pulse_config_get()
{
    return sequencer_pulse_config;
}


// NOTE: Has debug messages incl.
// Configure all active state machines based on static array of clock
// configs.
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
                &sequencer_clock_config[i]
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
// Configure all active state machines based on static array of pulse
// configs.
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
        return;
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
                return;
            }

            debug_message_print_i(
                debug_status_local_func,
                "Internal Message: Starting to configure output state machine for channel %i\r\n",
                i
            );

            sequencer_output_sm_config(
                &sequencer_pulse_config[i]
            );

            debug_message_print_i(
                debug_status_local_func,
                "Internal Message: Finished configuring output state machine for channel %i\r\n",
                i
            );
        }
    }
}


// Get bit mask of all active state machiens for clock programs
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


// Get bit mask of all active state machiens for pulse programs
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
// Stall the synchronizer core until dma channels are empty.
// This has the effect of halting the program at this entry point so
// the synchronizer does not continue to execute later insructions.
// This is accomplished by stalling the program on the dma channel for
// the clock programs when software  triggers are used, or pulse
// configs when using external trigger.
// TODO: Validate that the stall of external triggers actually works
// as indented.
void sequencer_clock_sm_stall()
{
    uint32_t debug_status_local_func = debug_status_get();

    for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
    {
        // If a clock is not configured, then do not stall
        if (sequencer_clock_config[i].configured != true)
        {
            continue;
        }

        debug_message_print_i(
            debug_status_local_func,
            "Internal Message: Entering stall for clock id: %i\r\n",
            i
        );
        
        // Check if a dma channel is busy or if transmit fifo is empty.
        // If dma channel is not busy and transmit fifo is empty, we can
        // assume that the clock program is finished and undo the stall.
        while(
            (dma_channel_is_busy(sequencer_clock_config[i].dma_chan) ||
            !pio_sm_is_tx_fifo_empty(
                sequencer_clock_config[i].pio,
                sequencer_clock_config[i].sm
            )) &&
            (sequencer_status_get() != ABORT_REQUESTED)
        ) {
            // Check status every 100 microseconds / 10 kHz
            sleep_us(100);
        }
    }
}


// For all active clock and pulse programs, free them.
// TODO: Move checks into sequencer free/unclaim functions; not here
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


// For all pulse channels, validate that each output channel is tied
// to only a single pulse channel. While pulse channels can modify output
// channels linked to other pulse channels, the same output channel cannot
// accept two state changes at the same time which would cause bus contention
// and excess jitter. To prevent this, no pulse channels can cross over at all.
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


// Check that pulse instructions are valid
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


// Validate the clock IDs to make sure we don't explode (because exploding sucks)
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


// Make sure pulse id is within range (e.g., [0..2])
bool pulse_id_validate(
    uint32_t pulse_id
) {
    // Make sure the internal pulse ID is supported
    for (uint32_t i = 0; i < PULSES_MAX; i++)
    {
        uint32_t supported_pulse_id = INTERNAL_PULSE_IDS[i];

        if (pulse_id == supported_pulse_id)
        {
            return 1;
        }
    }

    return 0;
}


// Make sure trigger id is within range
bool trigger_id_validate(
    uint32_t trigger_id
) {
    // Make sure the trigger ID is supported
    for (uint32_t i = 0; i < TRIGGERS_MAX; i++)
    {
        uint32_t supported_trigger_id = EXTERNAL_TRIGGER_IDS[i];

        if (trigger_id == supported_trigger_id)
        {
            return 1;
        }
    }

    return 0;
}


// Make sure clock mode is supported
bool validate_clock_mode(
    uint32_t clock_mode
) {
    if (clock_mode == CLOCK_SCPI_MODE_FREERUN ||
        clock_mode == CLOCK_SCPI_MODE_SEQUENCER
    ) {
        return 1;
    }

    return 0;
}


// Make sure trigger mode is supported
bool validate_trigger_mode(
    uint32_t trigger_mode
) {
    if (trigger_mode == CLOCK_TRIG_SOURCE_IMMEDIATE ||
        trigger_mode == CLOCK_TRIG_SOURCE_EDGE ||
        trigger_mode == CLOCK_TRIG_SOURCE_GATE
    ) {
        return 1;
    }

    return 0;
}


// Make sure trigger edge is supported
bool validate_trigger_edge(
    uint32_t trigger_edge
) {
    if (trigger_edge == CLOCK_TRIG_EDGE_POSITIVE ||
        trigger_edge == CLOCK_TRIG_EDGE_NEGATIVE
    ) {
        return 1;
    }

    return 0;
}


// Make sure trigger level is supported
bool validate_trigger_level(
    uint32_t trigger_level
) {
    if (trigger_level == CLOCK_GATE_LEVEL_HIGH ||
        trigger_level == CLOCK_GATE_LEVEL_LOW
    ) {
        return 1;
    }

    return 0;
}


// Set the clock type of a clock channel
bool sequencer_clock_mode_set(
    uint32_t clock_id,
    uint32_t requested_mode
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Check clock type is valid and supported
    if (!validate_clock_mode(requested_mode))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].clock_mode = requested_mode;

    return 1;
}


// Set the trigger type of a clock channel
bool sequencer_trigger_mode_set(
    uint32_t clock_id,
    uint32_t requested_mode
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Check clock type is valid and supported
    if (!validate_trigger_mode(requested_mode))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].trigger_source = requested_mode;

    return 1;
}


// Set the trigger edge of a clock channel
bool sequencer_trigger_edge_set(
    uint32_t clock_id,
    uint32_t requested_edge
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Check clock type is valid and supported
    if (!validate_trigger_edge(requested_edge))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].trigger_edge = requested_edge;

    return 1;
}


// Set the clock type of a clock channel
bool sequencer_trigger_level_set(
    uint32_t clock_id,
    uint32_t requested_level
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Check clock type is valid and supported
    if (!validate_trigger_level(requested_level))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].trigger_level = requested_level;

    return 1;
}


// Set the clock divider of a clock channel
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


// Set the clock divider of a pulse channel
bool pulse_divider_set(
    uint32_t pulse_id,
    uint32_t clock_divider_copy
) {
    // Validate pulse ID
    if(!pulse_id_validate(pulse_id))
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


// Choose whether or not this clock channel is going to be loaded into sm memory
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


// Set the external trigger pin based on pin ID for a clock channel
bool trigger_pin_set(
    uint32_t clock_id,
    uint32_t trigger_pin_id
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    // Validate trigger ID
    if(!trigger_id_validate(trigger_pin_id))
    {
        return 0;
    }

    sequencer_clock_config[clock_id].trigger_pin = EXTERNAL_TRIGGER_PINS[trigger_pin_id];

    return 1;
}


// Sets the amount of accepted external triggers for a clock channel
bool trigger_count_set(
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


// Set trigger skip for amount of external triggers to skip
bool trigger_skip_set(
    uint32_t clock_id,
    uint32_t trigger_skips
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

    sequencer_clock_insert_instructions_triggered_skip(
        &sequencer_clock_config[clock_id],
        trigger_skips
    );

    return 1;
}


// Set trigger delay between clock signal and pulse sequence fire signal
bool trigger_delay_set(
    uint32_t clock_id,
    uint32_t trigger_delay
) {

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    sequencer_clock_insert_instructions_triggered_delay(
        &sequencer_clock_config[clock_id],
        trigger_delay
    );

    return 1;
}


// Load clock reps and iter instructions to a clock channel
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


// Set unit offset (scaling factor) for a clock channel
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


// Set trigger unit offset (scaling factor) for a clock channel
bool clock_trigger_unit_offset_set(
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

    sequencer_clock_config[clock_id].unit_offset_trigger = units_offset;

    return 1;
}


// Reset clock channel to hardcoded defualt state
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


// Choose whether or not this pulse channel is loaded into sm memory
bool pulse_sequencer_state_set(
    uint32_t pulse_id,
    bool pulse_state
) {
    // Validate pulse ID
    if (!pulse_id_validate(pulse_id))
    {
        return 0;
    }

    sequencer_pulse_config[pulse_id].active = pulse_state;

    return 1;
}


// Set the clock channel trigger pin for a pulse channel
bool pulse_pin_clock_set(
    uint32_t pulse_id,
    uint32_t clock_id
) {

    // Validate pulse ID
    if(!pulse_id_validate(pulse_id))
    {
        return 0;
    }

    // Validate clock ID
    if(!clock_id_validate(clock_id))
    {
        return 0;
    }

    sequencer_pulse_config[pulse_id].clock_pin = INTERNAL_CLOCK_PINS[clock_id];

    return 1;
}


// Set unit offset (scaling factor) for pulse channel delay instructions
bool pulse_unit_offset_set(
    uint32_t pulse_id,
    double units_offset
) {
    const double eps = 0.000001;

    // Validate pulse ID
    if(!pulse_id_validate(pulse_id))
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


// Load state and delay instructions to pulse channel
bool pulse_instructions_load(
    uint32_t pulse_id,
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX]
) {

    // Validate pulse ID
    if(!pulse_id_validate(pulse_id))
    {
        return 0;
    }

    sequencer_output_insert_instructions(
        &sequencer_pulse_config[pulse_id],
        instructions
    );

    return 1;
}


// Reset pulse channel to hardcoded defualts
bool pulse_sequencer_state_reset(
    uint32_t pulse_id
) {
    // Validate pulse ID
    if(!pulse_id_validate(pulse_id))
    {
        return 0;
    }

    sequencer_output_config_reset(
        &sequencer_pulse_config[pulse_id]
    );

    return 1;
}