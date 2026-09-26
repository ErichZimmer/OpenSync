#pragma once

#include <stdint.h>

// Index: disabled, rising edge, falling edge.
extern const uint32_t PINS_TRIGGER_EXTERNAL[3];

// Index: disabled, active HIGH, active LOW.
extern const uint32_t PINS_TRIGGER_GATE[3];

extern const uint32_t PINS_OUTPUT[8];

extern const uint32_t PIN_CLOCK_T0;
extern const uint32_t PIN_CLOCK_INTERNAL;

extern const double CLOCK_CYCLE_NS;