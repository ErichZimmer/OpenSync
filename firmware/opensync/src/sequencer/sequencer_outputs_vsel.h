#pragma once
// NOTE: This file is not currently imported and the soruce file compiled until my PCB arrives for testing

#include "structs/pulse_scpi_config.h"


void sequencer_vsel_init(void);

void sequencer_vsel_update(
    const struct pulse_scpi_config* scpi
);