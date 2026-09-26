#pragma once

#include "structs/pulse_sequencer_config.h"
#include "structs/pulse_scpi_config.h"


int sequencer_program_outputs_add(
    PIO pio
);

void sequencer_program_outputs_remove(
    PIO pio, 
    uint offset
);

bool sequencer_outputs_init(
    struct pulse_sequencer_config* config,
    PIO pio, 
    uint sm, 
    uint offset, 
    uint pin_out, 
    uint pin_trigger
);

bool sequencer_outputs_configure(
    struct pulse_sequencer_config* config,
    const struct pulse_scpi_config* scpi
);

bool sequencer_outputs_build_instructions(
    struct pulse_sequencer_config* config,
    const struct pulse_scpi_config* scpi
);

void sequencer_outputs_sm_helper_init(
    struct pulse_sequencer_config* config
);

bool sequencer_outputs_dma_configure(
    struct pulse_sequencer_config* config
);

bool sequencer_outputs_sm_config(
    struct pulse_sequencer_config* config,
    const struct pulse_scpi_config* scpi
);

void sequencer_outputs_dma_free(
    struct pulse_sequencer_config* config
);

void sequencer_outputs_sm_free(
    struct pulse_sequencer_config* config
);

void sequencer_outputs_free(
    struct pulse_sequencer_config* config
);
