#include "sequencer_clock_t0.h"

#include <assert.h>

#include "hardware/pio.h"

#include "sequencer/sequencer_common.h"
#include "sequencer_pio_clock_t0_gated.pio.h"


static bool t0_pio_valid(PIO pio)
{
    return pio == pio0 || pio == pio1 || pio == pio2;
}


static bool t0_resources_owned(
    const struct clock_sequencer_config* config
) {
    return config -> configured || config -> sm_claimed;
}


// Called explicitly by core_1 for the program's lifetime, not per acquisition.
int sequencer_program_clock_t0_add(PIO pio)
{
    if (!t0_pio_valid(pio) ||
        !pio_can_add_program(pio, &sequencer_pio_clock_t0_gated_program)
    ) {
        return -1;
    }

    return (int) pio_add_program(
        pio,
        &sequencer_pio_clock_t0_gated_program
    );
}


void sequencer_program_clock_t0_remove(
    PIO pio, 
    uint offset
) {
    pio_remove_program(
        pio, 
        &sequencer_pio_clock_t0_gated_program, 
        offset
    );
}


// core_1 passes its runtime object and the selected SCPI object separately.
bool sequencer_clock_t0_configure(
    struct clock_sequencer_config* config,
    const struct pulse_scpi_config* scpi
) {
    if (config == NULL || 
        scpi == NULL || 
        t0_resources_owned(config) ||
        !t0_pio_valid(config -> pio) ||
        config -> program_offset > (uint)PIO_INSTRUCTION_COUNT -
            sequencer_pio_clock_t0_gated_program.length ||
        scpi -> clock_divider < 1u || 
        scpi -> clock_divider > 65500 ||
        scpi -> gate_mode > SCPI_GATE_CHANNEL || 
        scpi -> gate_level > 1u
    ) {
        return false;
    }

    config -> pin_out = PIN_CLOCK_T0;
    config -> pin_trigger = PIN_CLOCK_INTERNAL;
    config -> pin_gate = PINS_TRIGGER_GATE[
        scpi -> gate_mode == SCPI_GATE_OUTPUT ? scpi -> gate_level + 1u : 0u
    ];

    uint32_t pcounter = scpi -> pcounter;

    if (!pcounter)
        pcounter = 1;

    config -> pcounter = pcounter;
    config -> ocounter = scpi -> ocounter;
    config -> clock_divider = scpi -> clock_divider;
    config -> active = scpi -> state;
    config -> dma_chan = -1;
    config -> bcounter = 0u;     // Acquisition quota belongs to the internal clock.
    config -> delay = 0u;        // Timing comes from the internal-clock GPIO.
    config -> instructions = 0u; // T0 uses persistent GET registers, never DMA.
    
    return true;
}


void sequencer_clock_t0_sm_helper_init(
    PIO pio, 
    uint sm,
    uint offset,
    uint pin_out, 
    uint pin_trigger,
    uint pin_gate,
    uint clock_divider
) {
    assert(t0_pio_valid(pio));
    assert(sm < NUM_PIO_STATE_MACHINES);
    assert(offset <= (uint) PIO_INSTRUCTION_COUNT -
           sequencer_pio_clock_t0_gated_program.length);
    assert(pin_out < NUM_BANK0_GPIOS && pin_out < 32u);
    assert(pin_trigger < NUM_BANK0_GPIOS && pin_trigger < 32u);
    assert(pin_gate < NUM_BANK0_GPIOS && pin_gate < 32u);
    assert(pin_out != pin_trigger && pin_out != pin_gate && pin_trigger != pin_gate);
    assert(clock_divider >= 1u && clock_divider <= 65535u);

    pio_sm_config sm_config =
        sequencer_pio_clock_t0_gated_program_get_default_config(offset);
    sm_config_set_in_pins(&sm_config, pin_trigger);
    sm_config_set_jmp_pin(&sm_config, pin_gate);

    sm_config_set_set_pins(
        &sm_config, 
        pin_out, 
        1u
    );

    sm_config_set_in_shift(
        &sm_config, 
        false, 
        false, 
        32u
    );
    sm_config_set_out_shift(
        &sm_config, 
        false, 
        false, 
        32u
    );

    sm_config_set_clkdiv_int_frac(
        &sm_config, 
        clock_divider, 
        0u
    );

    pio_sm_init(
        pio, 
        sm, 
        offset, 
        &sm_config
    );

    const uint32_t output_mask = 1u << pin_out;

    pio_sm_set_pins_with_mask(
        pio, 
        sm, 
        0u, 
        output_mask
    );
    
    pio_sm_set_pindirs_with_mask(
        pio, 
        sm, 
        output_mask, 
        output_mask
    );
}


bool sequencer_clock_t0_sm_config(
    struct clock_sequencer_config* config,
    const struct pulse_scpi_config* scpi
) {
    if (!sequencer_clock_t0_configure(config, scpi) || !config -> active)
        return false;

    if (pio_sm_is_claimed(config -> pio, config -> sm))
        return false;

    // core_1 has already loaded the program and supplied its static offset.
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

    sequencer_clock_t0_sm_helper_init(
        config -> pio,
        config -> sm, 
        config -> program_offset,
        config -> pin_out,
        config -> pin_trigger, 
        config -> pin_gate,
        config -> clock_divider
    );

    // RP2350 TXGET registers retain raw duty counts across every cycle.
    config -> pio -> rxf_putget[config -> sm][0] = config -> pcounter;
    config -> pio -> rxf_putget[config -> sm][1] = config -> ocounter;
    config -> pio -> rxf_putget[config -> sm][2] = 0u;
    config -> pio -> rxf_putget[config -> sm][3] = 0u;

    // core_1 primes T0 at its GPIO wait before starting the internal clock.
    config -> configured = true;

    return true;
}


void sequencer_clock_t0_sm_free(
    struct clock_sequencer_config* config
) {
    if (config == NULL)
        return;

    if (config -> sm_claimed) {
        pio_sm_set_enabled(
            config -> pio, 
            config -> sm, 
            false
        );

        pio_sm_set_pins_with_mask(
            config -> pio, 
            config -> sm, 0u,
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


void sequencer_clock_t0_free(
    struct clock_sequencer_config* config
) {
    sequencer_clock_t0_sm_free(config);
}
