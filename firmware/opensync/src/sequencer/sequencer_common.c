#include "sequencer_common.h"

// Used for user input validation
const uint32_t OUT_MASK = ((1u << OUTPUT_PIN_COUNT) - 1) << OUTPUT_PIN_BASE;

// Sequence stuff
const uint32_t ITERATIONS_MAX = 500000;
const uint32_t CLOCK_DIVIDER_MAX = 65000;
const uint32_t INTERNAL_CLOCK_PINS[3] = {16, 17, 18};
const uint32_t EXTERNAL_TRIGGER_PINS[3] = {13, 14, 15};