#include "sequencer_common.h"

#include <stdint.h>


// Used for user input validation
const uint32_t OUT_MASK = ((1u << OUTPUT_PIN_COUNT) - 1) << OUTPUT_PIN_BASE;

// Sequence stuff
const uint32_t CLOCK_DIV_DEFAULT = 1;
const uint32_t SEQUENCE_FLAG_END = 0;
const uint32_t ITERATIONS_MAX = 500000;
const uint32_t TRIGGER_SKIPS_MAX = 500;
const uint32_t CLOCK_DIVIDER_MAX = 65500;
const uint32_t PULSE_INSTRUCTION_OFFSET = 4;
const uint32_t CLOCK_INSTRUCTION_OFFSET = 1;
const uint32_t INTERNAL_CLOCK_IDS[CLOCKS_MAX] = {0, 1, 2};
const uint32_t INTERNAL_CLOCK_PINS[CLOCKS_MAX] = {16, 17, 18};
const uint32_t EXTERNAL_TRIGGER_PINS[TRIGGERS_MAX] = {13};