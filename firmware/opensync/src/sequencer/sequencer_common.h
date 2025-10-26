#pragma once

#include <stdint.h>


enum {
    OUTPUT_PIN_BASE = 0,
    OUTPUT_PIN_COUNT = 12,
};

extern const uint32_t OUT_MASK;
extern const uint32_t CLOCK_DIV_DEFAULT;
extern const uint32_t SEQUENCE_FLAG_END;
extern const uint32_t ITERATIONS_MAX;
extern const uint32_t TRIGGER_SKIPS_MAX;
extern const uint32_t CLOCK_DIVIDER_MAX;
extern const uint32_t PULSE_INSTRUCTION_OFFSET;
extern const uint32_t CLOCK_INSTRUCTION_OFFSET;
extern const uint32_t INTERNAL_CLOCK_IDS[3];
extern const uint32_t INTERNAL_CLOCK_PINS[3];
extern const uint32_t EXTERNAL_TRIGGER_PINS[3];