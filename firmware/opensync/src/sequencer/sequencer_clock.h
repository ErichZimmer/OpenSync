#pragma once

#include <stdint.h>
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "structs/clock_config.h"

enum {
    OUTPUT_PIN_BASE = 0,
    OUTPUT_PIN_COUNT = 12,
};

extern const uint32_t OUT_MASK;

void sequencer_clocks_init(
    struct clock_config* config_array,
    PIO clock_pio
);

bool sequencer_clocks_configure(
    struct clock_config* config_array,
    uint32_t* clock_pins,
    uint32_t* trigger_pins,
    uint32_t num_clocks
);

bool sequencer_insert_instructions_internal(
    struct clock_config* config_array,
    uint32_t (*instructions)[CLOCK_INSTRUCTIONS_MAX],
    uint32_t num_clocks
);

bool sequencer_insert_instructions_triggered(
    struct clock_config* config_array,
    uint32_t (*instructions)[CLOCK_TRIGGERS_MAX],
    uint32_t num_clocks
);