#pragma once

#include "hardware/pio.h"
#include "structs/pulse_scpi_config.h"


bool sequencer_clock_triggers_init(
    PIO pio,
    const struct pulse_scpi_config* scpi,
    bool triggered
);

void sequencer_clock_triggers_deinit(
    const struct pulse_scpi_config* scpi,
    bool triggered
);

void sequencer_trigger_manual(
    uint pin_trigger,
    bool triggered
);
