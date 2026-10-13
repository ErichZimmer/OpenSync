#pragma once

#include <stdint.h>

// Index: internal software trigger, rising edge, falling edge.
extern const uint32_t PINS_TRIGGER_EXTERNAL[3];

// Index: disabled, active LOW, active HIGH.
extern const uint32_t PINS_TRIGGER_GATE[3];

extern const uint32_t PIN_CLOCK_T0;
extern const uint32_t PIN_CLOCK_INTERNAL;

extern const uint32_t PINS_OUTPUT[8];
extern const uint32_t PINS_OUTPUT_VSEL[8];

extern const uint32_t PINS_OUTPUT_LEDS[8];
extern const uint32_t PIN_TRIGGER_GATE_LED;
extern const uint32_t PIN_TRIGGER_EXT_LED;
extern const uint32_t PIN_STATUS_LED;

extern const double CLOCK_CYCLE_NS;
extern const uint32_t DIVIDER_MAX;
