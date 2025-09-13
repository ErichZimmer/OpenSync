#pragma once

#include <stdint.h>

enum {
    OUTPUT_PIN_BASE = 0,
    OUTPUT_PIN_COUNT = 12,
};

extern const uint32_t OUT_MASK;
extern const uint32_t ITERATIONS_MAX;
extern const uint32_t CLOCK_DIVIDER_MAX;
extern const uint32_t INTERNAL_CLOCK_PINS[3];
extern const uint32_t EXTERNAL_TRIGGER_PINS[3];