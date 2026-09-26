#include "sequencer_outputs.h"

#include <math.h>
#include <string.h>

#include "hardware/dma.h"

#include "sequencer/sequencer_common.h"
#include "sequencer_pio_output.pio.h"


static bool output_hardware_valid(
    const struct pulse_sequencer_config* config
) {
    return config != NULL &&
        config -> pin_out < NUM_BANK0_GPIOS && 
        config -> pin_trigger < NUM_BANK0_GPIOS;
}


// core_1 calls these once per PIO block and supplies the resulting offsets.
int sequencer_program_outputs_add(PIO pio)
{
    if (
        (pio != pio0 && pio != pio1 && pio != pio2) ||
        !pio_can_add_program(pio, &sequencer_pio_output_program)
    ) {
        return -1;
    }

    return (int)pio_add_program(
        pio, 
        &sequencer_pio_output_program
    );
}


void sequencer_program_outputs_remove(
    PIO pio,
    uint offset
) {
    pio_remove_program(
        pio, 
        &sequencer_pio_output_program, 
        offset
    );
}


bool sequencer_outputs_init(
    struct pulse_sequencer_config* config,
    PIO pio, 
    uint sm, 
    uint program_offset, 
    uint pin_out, 
    uint pin_trigger
) {
    if (config == NULL || config -> configured || config -> sm_claimed)
        return false;

    struct pulse_sequencer_config next = {
        .pio = pio,
        .sm = sm,
        .program_offset = program_offset,
        .pin_out = pin_out,
        .pin_trigger = pin_trigger,
        .pin_gate = PINS_TRIGGER_GATE[0],
        .clock_divider = 1u,
        .dma_chan = -1,
        .wcounter = 0u,
        .bcounter = 0u,
        .pcounter = 1u,
        .ocounter = 0u,
        .gate_type = 0u,
        .gate_enabled = true,
        .active = false,
        .configured = false,
        .sm_claimed = false
    };

    if (!output_hardware_valid(&next))
        return false;

    for (uint i = 0; i < PULSE_SEQUENCER_WORDS - 1u; ++i)
        next.instructions[i] = 1u;

    next.instructions[PULSE_SEQUENCER_WORDS - 1u] = 0u;

    *config = next;

    return true;
}


bool sequencer_outputs_build_instructions(
    struct pulse_sequencer_config* config,
    const struct pulse_scpi_config* scpi
) {
    if (config == NULL || 
        scpi == NULL || 
        config -> configured || 
        config -> sm_claimed ||
        scpi -> clock_divider == 0u || 
        scpi -> clock_divider > 65500u
    ) {
        return false;
    }

    uint32_t instructions[PULSE_SEQUENCER_WORDS] = {0};

    for (uint i = 0; i < PULSE_SEQUENCER_SEGMENTS; ++i) {
        const double state = scpi -> state_buffer[i];
        const double seconds = scpi -> delay_buffer[i];

        if ((state != 0.0 && 
            state != 1.0) || 
            !isfinite(seconds) || 
            seconds < 0.0
        ) {
            return false;
        }

        const double delay_ns = seconds * 1e9;
        double cycles = round(
            delay_ns / (CLOCK_CYCLE_NS * scpi -> clock_divider)
        );

        // Adjust for cycle skew (10 clock cycles)
        // Note: if there are less than 11 cycles delay, it will intentionally fail.
        if (cycles >= 1.0)
            cycles = cycles - 10.0;
        
        if (!isfinite(cycles) || 
            cycles < 0.0 || 
            cycles > PULSE_SEQUENCER_DELAY_MAX
        ) {
            return false;
        }

        uint32_t delay = (uint32_t)cycles;

        if (delay == 0u)
            delay = 1u;

        instructions[i] = ((uint32_t)state << 31u) | delay;
    }

    // Reject a longer waveform instead of silently discarding SCPI entries.
    const uint scpi_segments = sizeof(scpi -> delay_buffer) / sizeof(scpi -> delay_buffer[0]);
    
    for (uint i = PULSE_SEQUENCER_SEGMENTS; i < scpi_segments; ++i) {
        if (scpi -> state_buffer[i] != 0.0f || 
            scpi -> delay_buffer[i] != 0.0f
        ) {
            return false;
        }
    }

    // Eight words form the 32-byte DMA ring.
    // The final instruction after the termination flag is used for
    // off duty cycle counter.
    instructions[PULSE_SEQUENCER_SEGMENTS] = 0u;
    instructions[PULSE_SEQUENCER_WORDS - 1u] = scpi->ocounter << 1u;

    memcpy(
        config -> instructions, 
        instructions, 
        sizeof(instructions)
    );

    return true;
}


bool sequencer_outputs_configure(
    struct pulse_sequencer_config* config,
    const struct pulse_scpi_config* scpi
) {
    if (!output_hardware_valid(config) || 
        scpi == NULL || 
        config -> configured ||
        config -> sm_claimed || 
        config -> dma_chan != -1 ||
        scpi -> clock_divider == 0u || 
        scpi -> clock_divider > 65535u ||
        scpi -> gate_mode > SCPI_GATE_OUTPUT ||
        scpi -> gate_level > SCPI_GATE_HIGH
    ) {
        return false;
    }

    // SCPI interface needs to be offset by 1 due to 0 representing disabled
    const uint pin_gate = PINS_TRIGGER_GATE[
        (config -> gate_enabled && scpi -> gate_mode != SCPI_GATE_DISABLED)
            ? scpi -> gate_level + 1u : 0u
    ];

    // Same thing for output channel sync source, subtract 1 for outptu channels.
    uint pin_trigger = (scpi -> sync == 0)
        ? PIN_CLOCK_T0 : PINS_OUTPUT[scpi -> sync - 1];

    if (pin_gate == config -> pin_out)
        return false;

    // Validate and build the complete buffer before changing runtime settings.
    if (!sequencer_outputs_build_instructions(config, scpi))
        return false;

    uint32_t pcounter = scpi -> pcounter;

    if (!pcounter)
        pcounter = 1;

    config -> pin_trigger = pin_trigger;
    config -> pin_gate = pin_gate;
    config -> gate_type = scpi -> gate_mode == SCPI_GATE_OUTPUT ? 1u : 0u;
    config -> wcounter = scpi -> wcounter;
    config -> bcounter = scpi -> bcounter;
    config -> pcounter = pcounter;
    config -> ocounter = scpi -> ocounter;
    config -> clock_divider = scpi -> clock_divider;
    config -> active = scpi -> state;

    return true;
}


void sequencer_outputs_sm_helper_init(
    struct pulse_sequencer_config* config
) {
    pio_sm_config sm_config =
        sequencer_pio_output_program_get_default_config(config -> program_offset);

    sm_config_set_in_pins(
        &sm_config, 
        config -> pin_trigger
    );

    sm_config_set_jmp_pin(
        &sm_config, 
        config -> pin_gate
    );

    sm_config_set_set_pins(
        &sm_config, 
        config -> pin_out, 
        1u
    );

    sm_config_set_out_pins(
        &sm_config, 
        config -> pin_out, 
        1u
    );

    sm_config_set_out_shift(
        &sm_config, 
        false, 
        false, 
        config -> gate_type ? 1u : 32u
    );

    sm_config_set_in_shift(
        &sm_config, 
        false,
        false, 
        32u
    );

    sm_config_set_clkdiv_int_frac(
        &sm_config,
        config -> clock_divider,
        0u
    );

    pio_sm_init(
        config -> pio,
        config -> sm,
        config -> program_offset + sequencer_pio_output_offset_initial_skip_entry,
        &sm_config
    );

    const uint32_t output_mask = 1u << config -> pin_out;

    pio_sm_set_pins_with_mask(
        config -> pio, 
        config -> sm, 
        0u, 
        output_mask
    );

    pio_sm_set_consecutive_pindirs(
        config -> pio, 
        config -> sm, 
        config -> pin_out, 
        1u, 
        true
    );

    pio_gpio_init(
        config -> pio, 
        config -> pin_out
    );

    pio_sm_put(
        config -> pio,
        config -> sm,
        config -> pcounter - 1u
    );

    pio_sm_exec_wait_blocking(
        config -> pio,
        config -> sm,
        pio_encode_pull(false, true)
    );

    pio_sm_exec_wait_blocking(
        config -> pio,
        config -> sm,
        pio_encode_mov(pio_isr, pio_osr)
    );

    // Initial skips use Y once. Empty OSR before enabling DMA or the SM.
    pio_sm_put(
        config -> pio, 
        config -> sm, 
        config -> wcounter
    );

    pio_sm_exec_wait_blocking(
        config -> pio, 
        config -> sm, 
        pio_encode_pull(false, true)
    );

    pio_sm_exec_wait_blocking(
        config -> pio, 
        config -> sm, 
        pio_encode_mov(pio_y, pio_osr)
    );

    pio_sm_exec_wait_blocking(
        config -> pio, 
        config -> sm, 
        pio_encode_out(pio_null, 32u)
    );
}


bool sequencer_outputs_dma_configure(
    struct pulse_sequencer_config* config
) {
    if (config == NULL || 
        !config -> sm_claimed || 
        config -> configured ||
        config -> dma_chan != -1
    ) {
        return false;
    }

    uint64_t transfer_count = 
        (uint64_t) config -> bcounter * PULSE_SEQUENCER_WORDS;

    // RP2350 finite DMA transfer counts use 28 bits.
    if (transfer_count > 0x0fffffffu)
        return false;

    // If bcounter is zero, set transfers to infinite
    if (!transfer_count)
        transfer_count = UINT32_MAX;

    config -> dma_chan = dma_claim_unused_channel(false);

    if (config -> dma_chan < 0)
        return false;

    dma_channel_config dma_config = dma_channel_get_default_config(config -> dma_chan);
    
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);

    channel_config_set_dreq(
        &dma_config,
        pio_get_dreq(config -> pio, config -> sm, true)
    );

    channel_config_set_ring(
        &dma_config, 
        false, 
        PULSE_SEQUENCER_RING_BITS
    );

    channel_config_set_chain_to(
        &dma_config, 
        (uint) config -> dma_chan
    );

    dma_channel_configure(
        config -> dma_chan, &dma_config,
        &config -> pio -> txf[config -> sm],
        config -> instructions,
        (uint32_t) transfer_count, // Eight words per ON waveform.
        true        // Prefill while this channel's SM remains disabled.
    );

    return true;
}


bool sequencer_outputs_sm_config(
    struct pulse_sequencer_config* config,
    const struct pulse_scpi_config* scpi
) {
    if (!sequencer_outputs_configure(config, scpi) || !config -> active)
        return false;

    if (pio_sm_is_claimed(config -> pio, config -> sm))
        return false;

    pio_sm_claim(
        config -> pio, 
        config -> sm
    );

    config -> sm_claimed = true;

    pio_sm_set_enabled(
        config -> pio, 
        config -> sm, 
        false
    );

    sequencer_outputs_sm_helper_init(config);

    if (!sequencer_outputs_dma_configure(config)) {
        sequencer_outputs_sm_free(config);

        return false;
    }
    config -> configured = true;

    return true; 
}


void sequencer_outputs_dma_free(
    struct pulse_sequencer_config* config
) {
    if (config == NULL || 
        !config -> sm_claimed ||
        config -> dma_chan < 0
    ) {
        return;
    }

    pio_sm_set_enabled(
        config -> pio, 
        config -> sm, 
        false
    );

    // SDK cleanup clears EN before aborting, as required by RP2350-E5.
    dma_channel_cleanup(config -> dma_chan);
    dma_channel_unclaim(config -> dma_chan);

    config -> dma_chan = -1;
    config -> configured = false;
}


void sequencer_outputs_sm_free(
    struct pulse_sequencer_config* config
) {
    if (config == NULL)
        return;

    sequencer_outputs_dma_free(config);

    if (config -> sm_claimed) {
        pio_sm_set_enabled(
            config -> pio, 
            config -> sm, 
            false
        );

        pio_sm_set_pins_with_mask(
            config -> pio, 
            config -> sm, 
            0u, 
            1u << config -> pin_out
        );

        pio_sm_drain_tx_fifo(
            config -> pio, 
            config -> sm
        );

        pio_sm_clear_fifos(
            config -> pio, 
            config -> sm
        );
        
        pio_sm_restart(
            config -> pio, 
            config -> sm
        );

        pio_sm_unclaim(
            config -> pio, 
            config -> sm
        );

        config -> sm_claimed = false;
    }

    config -> configured = false;
}


void sequencer_outputs_free(
    struct pulse_sequencer_config* config
) {
    sequencer_outputs_sm_free(config);
}
