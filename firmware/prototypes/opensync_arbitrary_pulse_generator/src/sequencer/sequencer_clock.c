#include "sequencer_clock.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include <stdint.h>

#include "structs/clock_config.h"
#include "sequencer_common.h"

#include "sequencer_pio_clock_freerun.pio.h"
#include "sequencer_pio_clock_triggered_rising.pio.h"
#include "sequencer_pio_clock_triggered_falling.pio.h"
#include "sequencer_pio_clock_gated_high.pio.h"
#include "sequencer_pio_clock_gated_low.pio.h"


// Sequence stuff
uint32_t CLOCK_INSTRUCTIONS_DEFAULT[CLOCK_INSTRUCTIONS_MAX] = {0};
uint32_t CLOCK_TRIGGERS_DEFAULT[CLOCK_TRIGGERS_MAX] = {0};


// Helpfer function to get the correct internal clock mode
// NOTE: This is ugly as fuck
// TODO: Refactor this mess
bool clock_sequencer_map_mode(
    struct clock_config* config,
    uint32_t* internal_mode
) {
    const uint32_t clock_mode    = config -> clock_mode;
    const uint32_t trigger_mode  = config -> trigger_source;
    const uint32_t trigger_edge  = config -> trigger_edge;
    const uint32_t trigger_level = config -> trigger_level;

    if (clock_mode == CLOCK_SCPI_MODE_FREERUN)
    {
        if (trigger_mode == (uint32_t) CLOCK_TRIG_SOURCE_IMMEDIATE)
        {
            *internal_mode = (uint32_t) CLOCK_FREERUN;
        }
//        else if (trigger_mode == CLOCK_TRIG_SOURCE_EDGE)
//        {
//            if (trigger_edge == CLOCK_TRIG_EDGE_POSITIVE)
//            {
//                *internal_mode = (uint32_t) CLOCK_FREERUN_START_RISING;
//            }
//            else
//            {
//                *internal_mode = (uint32_t) CLOCK_FREERUN_START_FALLING;
//            }
//        }
        else if (trigger_mode == CLOCK_TRIG_SOURCE_GATE)
        {
            if (trigger_level == (uint32_t) CLOCK_GATE_LEVEL_HIGH)
            {
                *internal_mode = (uint32_t) CLOCK_TRIGGERED_HIGH;
            }
            else
            {
                *internal_mode = (uint32_t) CLOCK_TRIGGERED_LOW;
            }
        }
        else
        {
            return false;
        }
    }
    else if (clock_mode == CLOCK_SCPI_MODE_SEQUENCER)
    {
        // Currently, Clock mode of sequence and trigger mode of immediate is undefined
        if (trigger_mode == CLOCK_TRIG_SOURCE_IMMEDIATE)
        {
            return false;
        }
        else if (trigger_mode == CLOCK_TRIG_SOURCE_EDGE)
        {
            if (trigger_edge == CLOCK_TRIG_EDGE_POSITIVE)
            {
                *internal_mode = (uint32_t) CLOCK_TRIGGERED_RISING;
            }
            else
            {
                *internal_mode = (uint32_t) CLOCK_TRIGGERED_FALLING;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}


uint sequencer_program_clock_add(
    PIO pio_clock,
    uint32_t clock_type
) {
    switch (clock_type)
    {
        case CLOCK_FREERUN:
            return pio_add_program(
                pio_clock,
                &sequencer_pio_clock_freerun_program
            );
        case CLOCK_TRIGGERED:
        case CLOCK_TRIGGERED_RISING:
            return pio_add_program(
                pio_clock,
                &sequencer_pio_clock_triggered_rising_program
            );
        case CLOCK_TRIGGERED_FALLING:
            return pio_add_program(
                pio_clock,
                &sequencer_pio_clock_triggered_falling_program
            );
        case CLOCK_TRIGGERED_HIGH:
            return pio_add_program(
                pio_clock,
                &sequencer_pio_clock_gated_high_program
            );
        case CLOCK_TRIGGERED_LOW:
            return pio_add_program(
                pio_clock,
                &sequencer_pio_clock_gated_low_program
            );
        default:
            // We should never get to this point.
            break;
    }
    return 0;
}


void sequencer_program_clock_remove(
    PIO pio_clock, 
    uint offset,
    uint32_t clock_type
) {
    switch (clock_type)
    {
        case CLOCK_FREERUN:
            pio_remove_program(
                pio_clock,
                &sequencer_pio_clock_freerun_program,
                offset
            );
            break;

        case CLOCK_TRIGGERED:
        case CLOCK_TRIGGERED_RISING:
            pio_remove_program(
                pio_clock,
                &sequencer_pio_clock_triggered_rising_program,
                offset
            );
            break;

        case CLOCK_TRIGGERED_FALLING:
            pio_remove_program(
                pio_clock,
                &sequencer_pio_clock_triggered_falling_program,
                offset
            );
            break;

        case CLOCK_TRIGGERED_HIGH:
            pio_remove_program(
                pio_clock,
                &sequencer_pio_clock_gated_high_program,
                offset
            );
            break;

        case CLOCK_TRIGGERED_LOW:
            pio_remove_program(
                pio_clock,
                &sequencer_pio_clock_gated_low_program,
                offset
            );
            break;

        default:
            // We should never get to this point.
            break;
    }
}


void sequencer_clocks_init(
    struct clock_config* config_array,
    PIO clock_pio
) {
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        config_array[i].pio = clock_pio;
        config_array[i].sm = i;
        config_array[i].dma_chan = i;
        config_array[i].program_offset = 0;
        config_array[i].clock_mode = CLOCK_MODE_DEFAULT;
        config_array[i].trigger_source = TRIGGER_MODE_DEFAULT;
        config_array[i].trigger_edge = TRIGGER_EDGE_DEFAULT;
        config_array[i].trigger_level = TRIGGER_GATE_DEFAULT;
        config_array[i].clock_pin = INTERNAL_CLOCK_PINS[i];
        config_array[i].trigger_pin = EXTERNAL_TRIGGER_PINS[0]; // All clocks should default to same trigger pin
        config_array[i].trigger_reps = 0;
        config_array[i].clock_divider = CLOCK_DIV_DEFAULT;
        config_array[i].unit_offset = CLOCK_UNITS_OFFSET_DEFAULT;
        config_array[i].unit_offset_trigger = PULSE_UNITS_OFFSET_DEFAULT;
        config_array[i].active = false;
        config_array[i].configured = false;
    }
}


void sequencer_clock_configure(
    struct clock_config* config,
    uint32_t clock_pin,
    uint32_t trigger_pin,
    uint32_t trigger_reps
) {
    config -> clock_pin = clock_pin;
    config -> trigger_pin = trigger_pin;
    config -> trigger_reps = trigger_reps;
}


void sequencer_clock_insert_instructions_internal(
    struct clock_config* config,
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX]
) {
    for (uint32_t i = 0; i < CLOCK_INSTRUCTIONS_MAX; i++)
    {
        config -> instructions[i] = instructions[i];
    }
}


void sequencer_clock_insert_instructions_triggered(
    struct clock_config* config,
    uint32_t instructions[CLOCK_TRIGGERS_MAX]
) {
    for (uint32_t i = 0; i < CLOCK_TRIGGERS_MAX; i++)
    {
        config -> trigger_config[i] = instructions[i];
    }
}


void sequencer_clock_insert_instructions_triggered_skip(
    struct clock_config* config,
    uint32_t skip
) {
    const size_t INDEX_SKIP = 0;
    config -> trigger_config[INDEX_SKIP] = skip;
}


void sequencer_clock_insert_instructions_triggered_delay(
    struct clock_config* config,
    uint32_t delay
) {
    const size_t INDEX_DELAY = 1;
    config -> trigger_config[INDEX_DELAY] = delay;
}


void sequencer_clock_config_reset(
    struct clock_config* config
) {
    sequencer_clock_insert_instructions_internal(
        config,
        CLOCK_INSTRUCTIONS_DEFAULT
    );

    sequencer_clock_insert_instructions_triggered(
        config,
        CLOCK_TRIGGERS_DEFAULT
    );

    config -> trigger_pin = EXTERNAL_TRIGGER_PINS[0];
    config -> trigger_reps = 0;
    config -> clock_divider = CLOCK_DIV_DEFAULT;
    config -> unit_offset = CLOCK_UNITS_OFFSET_DEFAULT;
    config -> unit_offset_trigger = PULSE_UNITS_OFFSET_DEFAULT;
    config -> clock_mode = CLOCK_MODE_DEFAULT;
    config -> trigger_source = TRIGGER_MODE_DEFAULT;
    config -> trigger_edge = TRIGGER_EDGE_DEFAULT;
    config -> trigger_level = TRIGGER_GATE_DEFAULT;
    config -> active = false;
}


void sequencer_clock_dma_configure(
    struct clock_config* config,
    uint32_t clock_type
) {
    const uint RING_BUFF_SIZE_POWER = 3; //log(2)(CLOCK_TRIGGERS_MAX * 4)

    config -> dma_chan = dma_claim_unused_channel(true);
    
    dma_channel_config dma_config = dma_channel_get_default_config(config -> dma_chan);

    // Enable read increment and disable write increment
	channel_config_set_read_increment(
        &dma_config, 
        true
    );
	channel_config_set_write_increment(
        &dma_config, 
        false
    );

    // Set read increment size
    channel_config_set_transfer_data_size(
        &dma_config, 
        DMA_SIZE_32
    );

    // Set data transfer request signal
	channel_config_set_dreq(
        &dma_config, 
        pio_get_dreq(
            config -> pio, 
            config -> sm, 
            true
        )
    );
    
    switch(clock_type)
    {
        case CLOCK_FREERUN:
        case CLOCK_TRIGGERED_HIGH:
        case CLOCK_TRIGGERED_LOW:
            // Start dma with the selected channel, generated config
            dma_channel_configure(
                config -> dma_chan,
                &dma_config,
                &config -> pio->txf[config -> sm], // Source pointer
                config -> instructions, // Instruction read address
                CLOCK_INSTRUCTIONS_MAX, // Number of instructions
                true // Do not start automatically
            );
            break;

        case CLOCK_TRIGGERED:
        case CLOCK_TRIGGERED_RISING:
        case CLOCK_TRIGGERED_FALLING:
            channel_config_set_ring(
                &dma_config,
                false,
                RING_BUFF_SIZE_POWER
            );

            // Start dma with the selected channel, generated config
            dma_channel_configure(
                config -> dma_chan,
                &dma_config,
                &config -> pio->txf[config -> sm], // Source pointer
                config -> trigger_config, // Instruction read address
                CLOCK_TRIGGERS_MAX * config -> trigger_reps, // Number of instructions * reps to perform
                true // Start transfers immediately
            );

            break;

        default:
            // We should never get to this point.
            break;
    }
}


void sequencer_freerun_sm_helper_init(
    PIO pio, uint sm, 
    uint offset, 
    uint pin_out,
    uint clock_divider
) {
    // Initialize gpio pin
    pio_gpio_init(pio, pin_out);

    // Set output pin direction as output
	pio_sm_set_consecutive_pindirs(
        pio, sm,
		pin_out,
		1, // only one pin is used
        true
    );

    // Get config for pio state machine
	pio_sm_config config = sequencer_pio_clock_freerun_program_get_default_config(offset);

    // Set output pins of config to output pins
	sm_config_set_set_pins(
        &config,
        pin_out, 
        1 // only one pin is used
    );

    // Setup clock clock divider
    sm_config_set_clkdiv(
        &config,
        (float) clock_divider
    );

    pio_sm_init(
        pio, sm,
        offset,
        &config
    );
}


void sequencer_triggered_sm_helper_init(
    PIO pio, uint sm, 
    uint offset, 
    uint pin_out,
    uint pin_trig, 
    uint clock_divider,
    uint32_t clock_type
) {
    // Initialize GPIO pin
    pio_gpio_init(pio, pin_out);
    pio_gpio_init(pio, pin_trig);

    // Set pin directions
    pio_sm_set_consecutive_pindirs(
        pio, sm,
		pin_out,
		1, // only one pin is used
        true // output direction
    );

	pio_sm_set_consecutive_pindirs(
        pio, sm,
		pin_trig,
		1, // only one pin is used
        false // input direction
    );

    // Defualt init (don't really care what, just as long as it isn't null)
    pio_sm_config config = sequencer_pio_clock_triggered_rising_program_get_default_config(offset);

    switch (clock_type)
    {
        case CLOCK_TRIGGERED:
        case CLOCK_TRIGGERED_RISING:
            config = sequencer_pio_clock_triggered_rising_program_get_default_config(offset);
            break;
        
        case CLOCK_TRIGGERED_FALLING:
            config = sequencer_pio_clock_triggered_falling_program_get_default_config(offset);
            break;

        case CLOCK_TRIGGERED_HIGH:
            config = sequencer_pio_clock_gated_high_program_get_default_config(offset);
            break;
        
        case CLOCK_TRIGGERED_LOW:
            config = sequencer_pio_clock_gated_low_program_get_default_config(offset);
            break;

        default:
            // We should never get to this point...
            return;

    }

    // Set output pins of config to output pins
	sm_config_set_set_pins(
        &config,
        pin_out, 
        1 // only one pin is used
    );

    // Set trigger pins of config to input pins
    if (clock_type == CLOCK_TRIGGERED_HIGH ||
        clock_type == CLOCK_TRIGGERED_LOW)
    {
        sm_config_set_jmp_pin(
            &config,
            pin_trig
        );
    }
    else
    {
        sm_config_set_in_pins(
            &config,
            pin_trig
        );
    }

    // Setup autopull for 32 bit words
    sm_config_set_out_shift(
        &config, 
        true, 
        true, 
        32
    );

    // Setup clock clock divider
    sm_config_set_clkdiv(
        &config,
        (float) clock_divider
    );

    pio_sm_init(
        pio, sm,
        offset,
        &config
    );
}


// NOTE: Claims both state machine and PIO memory
// NOTE: sets configured to true
void sequencer_clock_sm_config(
    struct clock_config* config
) {
    // Get current program mode to configure
    uint32_t clock_type = 0;

    const bool success = clock_sequencer_map_mode(
        config,
        &clock_type
    );

    // If the mappings to PIO programs fail, early exit.
    // This will cause configured to remain false and the clock
    // channel to be ignored.
    if (!success)
    {
        return;
    }

    // Claim unused state machine
    pio_claim_sm_mask(
        config -> pio,
        1u << config -> sm
    );

    // Claim unused PIO memory and add clock program
    config -> program_offset = sequencer_program_clock_add(
        config -> pio,
        clock_type
    );

    // Make sure the state machine is disabled
	pio_sm_set_enabled(
        config -> pio, 
        config -> sm,
        false
    );

    // Reset the state machine to default state
    pio_sm_restart(
        config -> pio, 
        config -> sm
    );

    switch(clock_type)
    {
        case CLOCK_FREERUN:
            sequencer_freerun_sm_helper_init(
                config -> pio, 
                config -> sm,
                config -> program_offset,
                config -> clock_pin,
                config -> clock_divider
            );
            break;

        case CLOCK_TRIGGERED:
        case CLOCK_TRIGGERED_RISING:
        case CLOCK_TRIGGERED_FALLING:
        case CLOCK_TRIGGERED_HIGH:
        case CLOCK_TRIGGERED_LOW:
            sequencer_triggered_sm_helper_init(
                config -> pio,
                config -> sm,
                config -> program_offset,
                config -> clock_pin,
                config -> trigger_pin,
                config -> clock_divider,
                clock_type
            );
            break;

        default:
            // We should never get to this point...
            return;
    }

    sequencer_clock_dma_configure(
        config,
        clock_type
    );

    config -> configured = true;
}


void sequencer_clock_dma_free(
    struct clock_config* config
) {
    dma_channel_abort(
        config -> dma_chan
    );

    dma_channel_cleanup(
        config -> dma_chan
    );

    dma_channel_unclaim(
        config -> dma_chan
    );
}


// NOTE: Unclaims both state machine and PIO memory
void sequencer_clock_sm_free(
    struct clock_config* config
) {
    // Get current program mode to configure.
    // It is assumed that all programs are valid at this point.
    // No checks are done on clock_type...
    uint32_t clock_type = 0;

    const bool success = clock_sequencer_map_mode(
        config,
        &clock_type
    );

    // Ensure that all outputs are low
    pio_sm_set_pins(
        config -> pio,
        config -> sm,
        0 // 0 = low
    );

    pio_sm_drain_tx_fifo(
        config -> pio,
        config -> sm
    );

    pio_sm_clear_fifos(
        config -> pio,
        config -> sm
    );

    // Ensure that all outputs are low (again)
    pio_sm_set_pins(
        config -> pio,
        config -> sm,
        0 // 0 = low
    );

    // Unclaim state machine
    pio_sm_unclaim(
        config -> pio,
        config -> sm
    );

    // Unclaim PIO memory
    sequencer_program_clock_remove(
        config -> pio,
        config -> program_offset,
        clock_type
    );
}


// NOTE: sets configured to false
void sequencer_clock_free(
    struct clock_config* config
) {
    // If this resource has not been initialized, do not uninit it
    if (config -> configured == false)
    {
        return;
    }

    pio_sm_set_enabled(
        config -> pio,
        config -> sm,
        false
    );
    sequencer_clock_dma_free(config);
    sequencer_clock_sm_free(config);

    config -> configured = false;
}