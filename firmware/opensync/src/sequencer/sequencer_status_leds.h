#pragma once

// NOTE: This file is not currently imported and the soruce file compiled until my PCB arrives for testing
#include <stdint.h>

#include "structs/pulse_scpi_config.h"


void sequencer_leds_init(void);

void sequencer_leds_output_update(
    const struct pulse_scpi_config* scpi
);

void sequencer_leds_input_update(
    const struct pulse_scpi_config* scpi
);

void sequencer_leds_program_update(
    uint32_t status
);