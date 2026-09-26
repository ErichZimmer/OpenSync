#include "sequencer_common.h"


// GPIO 4 supplies the disabled/LOW input.
// The board supplies rising-edge-compatible trigger signals.
const uint32_t PINS_TRIGGER_EXTERNAL[3] = {
    4,      // Disabled
    19,     // External rising edge
    18      // External falling edge, conditioned by the board
};

// The board supplies both gate polarities.
// PIO always treats HIGH as the asserted gate state.
const uint32_t PINS_TRIGGER_GATE[3] = {
    4,      // Disabled
    21,     // External gate active LOW, conditioned by the board
    23      // External gate active HIGH
};

const uint32_t PINS_OUTPUT[8] = {8, 9, 10, 11, 12, 13, 14, 15};

const uint32_t PIN_CLOCK_T0 = 2;
const uint32_t PIN_CLOCK_INTERNAL = 3;

// Assumes a 250 MHz system clock.
// Effective SM cycle = CLOCK_CYCLE_NS * clock_divider.
const double CLOCK_CYCLE_NS = 4.0;