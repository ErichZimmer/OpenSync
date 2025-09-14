#pragma once

#include <stdint.h>
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "structs/clock_config.h"
#include "sequencer_common.h"

uint sequencer_program_freerun_add(
    PIO pio_clock
);

uint sequencer_program_triggered_add(
    PIO pio_clock
);

void sequencer_program_freerun_remove(
    PIO pio_clock, 
    uint offset
);

void sequencer_program_triggered_remove(
    PIO pio_clock, 
    uint offset
);

void sequencer_clocks_init(
    struct clock_config* config_array,
    PIO clock_pio
);

void sequencer_clock_configure(
    struct clock_config* config,
    uint32_t clock_pin,
    uint32_t trigger_pin
);

void sequencer_clock_insert_instructions_internal(
    struct clock_config* config,
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX]
);

void sequencer_clock_insert_instructions_triggered(
    struct clock_config* config,
    uint32_t instructions[CLOCK_TRIGGERS_MAX]
);

void sequencer_clock_config_reset(
    struct clock_config* config
);

void sequencer_clock_freerun_dma_configure(
    struct clock_config* config
);

void sequencer_clock_triggered_dma_configure(
    struct clock_config* config,
    uint32_t reps
);

void sequencer_clock_freerun_sm_config(
    struct clock_config* config,
    uint offset,
    uint pin_out,
    uint clock_divider
);

void sequencer_clock_triggered_sm_config(
    struct clock_config* config,
    uint offset,
    uint pin_out,
    uint pin_triggwer,
    uint clock_divider,
    uint32_t reps
);

void sequencer_clock_dma_free(
    struct clock_config* config
);

void sequencer_clock_sm_free(
    struct clock_config* config
);

void sequencer_clock_free(
    struct clock_config* config
);

