#pragma once

#include <stdint.h>
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer_common.h"

uint sequencer_program_output_add(
    PIO pio_clock
);

void sequencer_program_ouput_remove(
    PIO pio_clock, 
    uint offset
);

void sequencer_output_init(
    struct pulse_config* config_array,
    PIO clock_pio
);

void sequencer_output_configure(
    struct pulse_config* config,
    uint32_t clock_pin
);

void sequencer_output_insert_instructions(
    struct pulse_config* config,
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX]
);

void sequencer_output_config_reset(
    struct pulse_config* config
);

void sequencer_output_dma_configure(
    struct pulse_config* config,
    uint32_t reps
);

void sequencer_output_sm_config(
    struct pulse_config* config,
    uint offset,
    uint clock_divider,
    uint32_t reps
);

void sequencer_output_dma_free(
    struct pulse_config* config
);

void sequencer_output_sm_free(
    struct pulse_config* config
);

void sequencer_output_free(
    struct pulse_config* config
);

