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