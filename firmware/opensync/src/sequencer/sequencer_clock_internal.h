#pragma once

#include "structs/clock_sequencer_config.h"
#include "structs/pulse_scpi_config.h"


int sequencer_program_clock_internal_add(PIO pio);

void sequencer_program_clock_internal_remove(
    PIO pio, 
    uint offset
);

bool sequencer_clock_internal_configure(
    struct clock_sequencer_config* config,
    const struct pulse_scpi_config* scpi
);

void sequencer_clock_internal_sm_helper_init(
    PIO pio, 
    uint sm, 
    uint offset,
    uint pin_out, 
    uint pin_trigger, 
    uint pin_gate, 
    uint clock_divider
);

bool sequencer_clock_internal_dma_configure(
    struct clock_sequencer_config* config
);

bool sequencer_clock_internal_sm_config(
    struct clock_sequencer_config* config,
    const struct pulse_scpi_config* scpi
);

bool sequencer_clock_internal_finished(
    const struct clock_sequencer_config* config
);

void sequencer_clock_internal_dma_free(
    struct clock_sequencer_config* config
);

void sequencer_clock_internal_sm_free(
    struct clock_sequencer_config* config
);

void sequencer_clock_internal_free(
    struct clock_sequencer_config* config
);
