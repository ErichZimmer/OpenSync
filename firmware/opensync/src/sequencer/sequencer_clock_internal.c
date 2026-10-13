#include "sequencer_clock_internal.h"

#include <math.h>

#include "hardware/dma.h"

#include "sequencer/sequencer_common.h"
#include "structs/pulse_scpi_config.h"
#include "structs/clock_sequencer_config.h"

#include "sequencer_pio_clock_internal_triggered.pio.h"
#include "sequencer_pio_clock_internal_freerun.pio.h"


// core_1 loads the selected program during arming.
int sequencer_program_clock_internal_add(
    PIO pio,
    bool triggered
) {
    const struct pio_program* program = triggered
        ? &sequencer_pio_clock_internal_triggered_program
        : &sequencer_pio_clock_internal_freerun_program;

    if (!pio_can_add_program(pio, program))
        return -1;

    return (int) pio_add_program(
        pio,
        program
    );
}


void sequencer_program_clock_internal_remove(
    PIO pio, 
    uint offset,
    bool triggered
) {
    const struct pio_program* program = triggered
        ? &sequencer_pio_clock_internal_triggered_program
        : &sequencer_pio_clock_internal_freerun_program;

    pio_remove_program(
        pio, 
        program,
        offset
    );
}


bool sequencer_clock_internal_configure(
    struct clock_sequencer_config* config,
    const struct pulse_scpi_config* scpi,
    bool triggered
) {
    if (config == NULL || 
        scpi == NULL || 
        config -> configured || 
        config -> sm_claimed ||
        scpi -> clock_divider > DIVIDER_MAX ||
        scpi -> trig_mode >= SCPI_TRIGGER_MODE_END || 
        scpi -> trig_edge >= SCPI_TRIGGER_EDGE_END ||
        scpi -> gate_mode >= SCPI_GATE_MODE_END || 
        scpi -> gate_level >= SCPI_GATE_LEVEL_END
    ) {
        return false;
    }

    uint32_t clock_divider = scpi -> clock_divider;

    // If clock divider is zero (this is default behvior), then make it 1
     if (!clock_divider)
        clock_divider = 1u;

    const uint32_t events = scpi -> pcounter + scpi -> ocounter;

    const double delay_ns = (double) scpi -> period * 1e9;
    const uint64_t cycles = (uint32_t) round(delay_ns / (CLOCK_CYCLE_NS * clock_divider));
    const uint64_t delay_cycles = cycles - 8; // PIO instruction overhead

    if (events == 0 || 
        cycles < CLOCK_SEQUENCER_MIN_DELAY ||
        delay_cycles > UINT32_MAX
    ) {
        return false;
    }

    config -> dma_chan = -1;
    config -> bcounter = scpi -> bcounter;
    config -> pcounter = scpi -> pcounter;
    config -> ocounter = scpi -> ocounter;
    config -> delay = (uint32_t) delay_cycles;
    config -> instructions = 0;
    config -> clock_divider = clock_divider;
    config -> active = scpi -> state;
    config -> pin_out = PIN_CLOCK_INTERNAL;
    config -> pin_trigger = PINS_TRIGGER_EXTERNAL[triggered ? scpi -> trig_edge + 1u : 0u];
    config -> pin_gate = PINS_TRIGGER_GATE[
        (scpi -> gate_mode == SCPI_GATE_PULSE) ? scpi -> gate_level + 1u : 0u];
    
    return true;
}


void sequencer_clock_internal_sm_helper_init(
    PIO pio, 
    uint sm, 
    uint offset,
    uint pin_out,
    uint pin_trigger,
    uint pin_gate, 
    uint clock_divider,
    bool triggered
) {
    pio_sm_config sm_config = triggered
        ? sequencer_pio_clock_internal_triggered_program_get_default_config(offset)
        : sequencer_pio_clock_internal_freerun_program_get_default_config(offset);
    
    sm_config_set_sideset_pins(&sm_config, pin_out);
    sm_config_set_in_pins(&sm_config, pin_trigger);
    sm_config_set_jmp_pin(&sm_config, pin_gate);

    sm_config_set_out_shift(
        &sm_config, 
        true, 
        false, 
        32
    );
    sm_config_set_in_shift(
        &sm_config, 
        true, 
        false,
        32
    );

    sm_config_set_clkdiv_int_frac(
        &sm_config, 
        clock_divider, 
        0
    );

    pio_sm_init(
        pio, 
        sm, 
        offset, 
        &sm_config
    );

    pio_sm_set_pins_with_mask(
        pio, 
        sm, 
        0, 
        1u << pin_out
    );

    pio_sm_set_consecutive_pindirs(
        pio, 
        sm, 
        pin_out, 
        1, 
        true
    );
}


bool sequencer_clock_internal_dma_configure(
    struct clock_sequencer_config* config
) {
    if (config == NULL || 
        !config -> sm_claimed ||
        config -> configured ||
        config -> dma_chan != -1
    ) {
        return false;
    }

    config -> dma_chan = dma_claim_unused_channel(false);

    if (config -> dma_chan < 0)
        return false;

    dma_channel_config dma_config = dma_channel_get_default_config(config -> dma_chan);
    
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);

    const uint dreq = pio_get_dreq(
        config -> pio, 
        config -> sm, 
        true
    );
    
    channel_config_set_dreq(
        &dma_config, 
        dreq
    );

    channel_config_set_ring(
        &dma_config, 
        false, 
        2 // Ring buffer size
    );

    channel_config_set_chain_to(
        &dma_config, 
        (uint) config -> dma_chan
    );

    uint32_t transfers = config -> bcounter;

    // If bcounter is 0, then set infinite transfers
    if (!transfers)
        transfers = UINT32_MAX;

    dma_channel_configure(
        config -> dma_chan, &dma_config,
        &config -> pio->txf[config -> sm],
        &config -> instructions,
        transfers, // One word per group; UINT32_MAX selects RP2350 endless mode
        true       // Prefill while the state machine remains disabled
    );

    return true;
}


bool sequencer_clock_internal_sm_config(
    struct clock_sequencer_config* config,
    const struct pulse_scpi_config* scpi,
    bool triggered
) {
    if (!sequencer_clock_internal_configure(config, scpi, triggered) || !config -> active)
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

    // core_1 has loaded this run's program and supplied its offset.
    sequencer_clock_internal_sm_helper_init(
        config -> pio, 
        config -> sm, 
        config -> program_offset,
        config -> pin_out, 
        config -> pin_trigger,
        config -> pin_gate, 
        config -> clock_divider,
        triggered
    );

    // Seed ISR before starting DMA, so a burst word cannot replace the delay.
    pio_sm_put(
        config -> pio, 
        config -> sm, 
        config -> delay
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

    const uint32_t event_counter = config -> pcounter + config -> ocounter;
    config -> instructions = event_counter - 1u;
    
    if (!sequencer_clock_internal_dma_configure(config)) {
        sequencer_clock_internal_sm_free(config);

        return false;
    }
    
    config -> configured = true;

    return true; // core_1 starts it after all GPIO consumers are armed.
}


bool sequencer_clock_internal_finished(
    const struct clock_sequencer_config* config
) {
    if (config == NULL || 
        !config -> configured || 
        !config -> sm_claimed ||
        config -> dma_chan < 0
    ) {
        return false;
    }

    dma_channel_hw_t* dma = dma_channel_hw_addr(config -> dma_chan);
    
    if (dma_channel_is_busy(config -> dma_chan) || 
        dma->transfer_count != 0 ||
        (dma->ctrl_trig & DMA_CH0_CTRL_TRIG_AHB_ERROR_BITS)
    ) {
        return false;
    }

    // Prefetch can exhaust DMA before the last accepted trigger/burst.
    // Read FIFO before PC: once DMA is done, empty FIFO stays empty.
    if (!pio_sm_is_tx_fifo_empty(config -> pio, config -> sm))
        return false;

    // The pull instruction is the configured wrap target for either program.
    const uint instruction_pull =
        (config -> pio->sm[config -> sm].execctrl & PIO_SM0_EXECCTRL_WRAP_BOTTOM_BITS) >>
        PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB;

    return pio_sm_get_pc(
        config -> pio, config -> sm
    ) == instruction_pull;
}


void sequencer_clock_internal_dma_free(
    struct clock_sequencer_config* config
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

    // SDK cleanup disables the channel before aborting it (RP2350-E5).
    dma_channel_cleanup(config -> dma_chan);
    dma_channel_unclaim(config -> dma_chan);

    config -> dma_chan = -1;
    config -> configured = false;
}


void sequencer_clock_internal_sm_free(
    struct clock_sequencer_config* config
) {
    if (config == NULL)
        return;

    sequencer_clock_internal_dma_free(config);

    if (config -> sm_claimed) {
        pio_sm_set_enabled(
            config -> pio, 
            config -> sm, 
            false
        );

        pio_sm_set_pins_with_mask(
            config -> pio, 
            config -> sm, 
            0, 
            1u << config -> pin_out
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
    
    // core_1 removes the selected program after the SM and DMA are freed.
    config -> configured = false;
}


void sequencer_clock_internal_free(
    struct clock_sequencer_config* config
) {
    sequencer_clock_internal_sm_free(config);
}
