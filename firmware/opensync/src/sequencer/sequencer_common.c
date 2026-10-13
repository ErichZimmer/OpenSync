#include "sequencer_common.h"


// The board supplies rising-edge-compatible trigger signals.
const uint32_t PINS_TRIGGER_EXTERNAL[3] = {
    3,      // Internal software trigger
    20,     // External rising edge
    19      // External falling edge, conditioned by the board
};

// The board supplies both gate polarities.
// PIO always treats HIGH as the asserted gate state.
const uint32_t PINS_TRIGGER_GATE[3] = {
    4,      // Disabled
    21,     // External gate active LOW, conditioned by the board
    23      // External gate active HIGH
};

const uint32_t PIN_CLOCK_T0 = 1;
const uint32_t PIN_CLOCK_INTERNAL = 2;

// TODO: Once PCB is obtained, set these to the actual pin numbers
const uint32_t PINS_OUTPUT[8]       = {8, 9, 10, 11, 12, 13, 14, 15};
const uint32_t PINS_OUTPUT_VSEL[8]  = {40, 41, 42, 43, 44, 45, 46, 47};
const uint32_t PINS_OUTPUT_LEDS[8]  = { 5,  6,  7,  8,  9, 10, 11, 12};
const uint32_t PIN_TRIGGER_GATE_LED = 14;
const uint32_t PIN_TRIGGER_EXT_LED  = 16;
const uint32_t PIN_STATUS_LED       = 17;

// Assumes a 250 MHz system clock.
// Effective SM cycle = CLOCK_CYCLE_NS * clock_divider.
const double CLOCK_CYCLE_NS = 4.0;
const uint32_t DIVIDER_MAX = 65500;
