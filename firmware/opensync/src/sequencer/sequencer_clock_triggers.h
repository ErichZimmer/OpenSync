#pragma once

#include "hardware/pio.h"
#include "structs/pulse_scpi_config.h"


bool sequencer_clock_triggers_init(
    PIO pio,
    const struct pulse_scpi_config* scpi
);

void sequencer_clock_triggers_deinit(
    const struct pulse_scpi_config* scpi
);
