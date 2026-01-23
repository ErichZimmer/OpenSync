#include "sequencer_output.h"

#include <stdint.h>
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer_common.h"

#include "sequencer_pio_pulse_sequencer.pio.h"


uint32_t PULSE_INSTRUCTIONS_DEFAULT[PULSE_INSTRUCTIONS_MAX] = {0};


uint sequencer_program_output_add(
    PIO pio_clock
) {
    return pio_add_program(
        pio_clock,
        &sequencer_pio_pulser_program
    );
}


void sequencer_program_ouput_remove(
    PIO pio_clock, 
    uint offset
) {
    pio_remove_program(
        pio_clock,
        &sequencer_pio_pulser_program,
        offset
    );
}


void sequencer_output_init(
    struct pulse_config* config_array,
    PIO clock_pio
) {
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        config_array[i].pio = clock_pio;
        config_array[i].sm = i;
        config_array[i].clock_pin = INTERNAL_CLOCK_PINS[0]; // Default to all using the same internal pin
        config_array[i].clock_divider = CLOCK_DIV_DEFAULT;
        config_array[i].unit_offset = PULSE_UNITS_OFFSET_DEFAULT;
        config_array[i].active = false;
        config_array[i].configured = false;
    }
}


void sequencer_output_configure(
    struct pulse_config* config,
    uint32_t clock_pin
) {
    config -> clock_pin = clock_pin;
}


void  sequencer_output_insert_instructions(
    struct pulse_config* config,
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX]
) {
    for (uint32_t i = 0; i < PULSE_INSTRUCTIONS_MAX; i++)
    {
        config -> instructions[i] = instructions[i];
    }
}


void sequencer_output_config_reset(
    struct pulse_config* config
) {
    sequencer_output_insert_instructions(
        config,
        PULSE_INSTRUCTIONS_DEFAULT
    );

    config -> clock_pin = INTERNAL_CLOCK_PINS[0];
    config -> clock_divider = CLOCK_DIV_DEFAULT;
    config -> unit_offset = PULSE_UNITS_OFFSET_DEFAULT;
    config -> active = false;
}


void sequencer_output_dma_configure(
    struct pulse_config* config
) {
    const uint RING_BUFF_SIZE_POWER = 8; //log(2)(PULSE_INSTRUCTIONS_MAX * 4)

    config -> dma_chan = dma_claim_unused_channel(true);
    
    dma_channel_config dma_config = dma_channel_get_default_config(config -> dma_chan);

    // Enable read increment and disable write increment
	channel_config_set_read_increment(&dma_config, true);
	channel_config_set_write_increment(&dma_config, false);

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
		&config -> instructions, // Instruction read address
		PULSE_INSTRUCTIONS_MAX * PULSE_ITERATIONS_MAX, // Number of instructions repeated the maximum amount of times
		true // Start transfers immediately
    );
}


void sequencer_output_sm_helper_init(
    PIO pio, uint sm, 
    uint offset, 
    uint pin_out_base,
    uint pin_out_count,
    uint pin_trig, 
    uint clock_divider
) {
    assert(clock_divider < 65535);

    // Initialize GPIO pins
    for(uint i = 0; i < pin_out_count; i++)
    {
		pio_gpio_init(pio, pin_out_base + i);
	}

    // Set output pins direction to output
    pio_sm_set_consecutive_pindirs(
        pio, sm,
		pin_out_base,
		pin_out_count,
        true
    );

    // Get config for pio state machine
	pio_sm_config config = sequencer_pio_pulser_program_get_default_config(offset);

    // Set output pins of config to output pins
	sm_config_set_out_pins(
        &config,
        pin_out_base, 
        pin_out_count
    );

    /* Note:
       The GPIO clock trigger pins would be already initialized by the
       clock PIO programs. DO NOT RECONFIGURE PIN DIRCTIONS!!!
    */
    
    // Set clock trigger pins of config to input pins
    sm_config_set_in_pins(
        &config,
        pin_trig
    );

    // Setup autopull for 32 bit words
    sm_config_set_out_shift(
        &config, 
        true, 
        true, 
        32
    );

    // Combine read and transmit FIFO to increase performance
    sm_config_set_fifo_join(
        &config, 
        PIO_FIFO_JOIN_TX
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


void sequencer_output_sm_config(
    struct pulse_config* config,
    uint offset
) {
    pio_claim_sm_mask(
        config -> pio,
        1u << config -> sm
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

    sequencer_output_sm_helper_init(
        config -> pio,
        config -> sm,
        offset,
        OUTPUT_PIN_BASE,
        OUTPUT_PIN_COUNT,
        config -> clock_pin,
        config -> clock_divider
    );

    sequencer_output_dma_configure(
        config
    );

    config -> configured = true;
}


void sequencer_output_dma_free(
    struct pulse_config* config
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


void sequencer_output_sm_free(
    struct pulse_config* config
) {
    // Ensure that all outputs are low
    pio_sm_set_pins(
        config -> pio,
        config -> sm,
        0 // 0 = low
    );

    // Deinit all outputs
    for (uint32_t i = 0; i < OUTPUT_PIN_COUNT; ++i)
    {
        gpio_deinit(
            OUTPUT_PIN_BASE + i
        );
    }

    pio_sm_drain_tx_fifo(
        config -> pio,
        config -> sm
    );

    pio_sm_clear_fifos(
        config -> pio,
        config -> sm
    );

    pio_sm_unclaim(
        config -> pio,
        config -> sm
    );
}


void sequencer_output_free(
    struct pulse_config* config
) {
    pio_sm_set_enabled(
        config -> pio,
        config -> sm,
        false
    );

    sequencer_output_dma_free(config);
    sequencer_output_sm_free(config);

    config -> configured = false;
}