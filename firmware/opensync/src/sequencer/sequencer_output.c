#include "sequencer_output.h"
#include "sequencer_pio_pulse_sequencer.pio.h"

uint32_t PULSE_INSTRUCTIONS_DEFAULT[PULSE_INSTRUCTIONS_MAX] = {0};
// uint32_t CLOCK_TRIGGERS_DEFAULT[CLOCK_TRIGGERS_MAX] = {0};


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
        config_array[i].clock_pin = INTERNAL_CLOCK_PINS[i];
        config_array[i].active = false;
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
    
    config -> active = false;
}


void sequencer_output_dma_configure(
    struct pulse_config* config,
    uint32_t reps
) {
    config -> dma_chan = dma_claim_unused_channel(true);
    
    dma_channel_config dma_config = dma_channel_get_default_config(config -> dma_chan);

    // Enable read increment and disable write increment
	channel_config_set_read_increment(&dma_config, true);
	channel_config_set_write_increment(&dma_config, false);

    // Set data transfer request signal
	channel_config_set_dreq(
        &dma_config, 
        pio_get_dreq(
            config -> pio, 
            config -> sm, 
            true
        )
    );

	// Start dma with the selected channel, generated config
	dma_channel_configure(
        config -> dma_chan,
        &dma_config,
		&config -> pio->txf[config -> sm], // Source pointer
		&config -> instructions, // Instruction read address
		PULSE_INSTRUCTIONS_MAX * reps, // Number of instructions * reps to perform
		true // Start transfers immediately
    );
}


void sequencer_output_sm_config(
    struct pulse_config* config,
    uint offset,
    uint clock_divider,
    uint32_t reps
) {

    // Make sure the state machine is disabled
	pio_sm_set_enabled(
        config -> pio, 
        config -> sm, 
        false
    );

    sequencer_pulser_sm_helper_init(
        config -> pio,
        config -> sm,
        offset,
        OUTPUT_PIN_BASE,
        OUTPUT_PIN_COUNT,
        config -> clock_pin,
        clock_divider
    );

    sequencer_output_dma_configure(
        config,
        reps
    );

    config -> active = true;
}


void sequencer_output_dma_free(
    struct pulse_config* config
) {
    dma_channel_abort(
        config -> dma_chan
    );

    dma_channel_unclaim(
        config -> dma_chan
    );
}


void sequencer_output_sm_free(
    struct pulse_config* config
) {
    pio_sm_drain_tx_fifo(
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
    sequencer_output_dma_free(config);
    sequencer_output_sm_free(config);
    sequencer_output_config_reset(config);
}