#pragma once

#include <stdint.h>

#include "structs/clock_config.h"


enum {
    OUTPUT_PIN_BASE = 0,
    OUTPUT_PIN_COUNT = 8,
};

extern const uint32_t OUT_MASK;
extern const uint32_t INTERNAL;
extern const uint32_t RISING_EDGE;
extern const uint32_t CLOCK_DIV_RES_HIGH;
extern const uint32_t CLOCK_DIV_RES_MED;
extern const uint32_t CLOCK_DIV_RES_LOW;
extern const uint32_t CLOCK_DIV_RES_VERY_LOW;
extern const uint32_t CLOCK_DIV_RES_VERY_VERY_LOW;
extern const uint32_t CLOCK_DIV_DEFAULT;
extern const uint32_t SEQUENCE_FLAG_END;
extern const uint32_t ITERATIONS_MAX;
extern const uint32_t TRIGGER_SKIPS_MAX;
extern const uint32_t CLOCK_DIVIDER_MAX;
extern const uint32_t PULSE_INSTRUCTION_OFFSET;
extern const uint32_t CLOCK_INSTRUCTION_OFFSET;
extern const uint32_t CLOCK_INSTRUCTION_MIN;
extern const uint32_t INTERNAL_CLOCK_IDS[CLOCKS_MAX];
extern const uint32_t INTERNAL_CLOCK_PINS[CLOCKS_MAX];
extern const uint32_t EXTERNAL_TRIGGER_PINS[TRIGGERS_MAX];
extern const double PULSE_UNITS_OFFSET_DEFAULT;
extern const double CLOCK_UNITS_OFFSET_DEFAULT;
