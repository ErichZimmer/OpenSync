#include "sequencer_common.h"

#include <stdint.h>

#include "structs/clock_config.h"
#include "structs/pulse_config.h"


// Used for user input validation
const uint32_t OUT_MASK = ((1u << OUTPUT_PIN_COUNT) - 1) << OUTPUT_PIN_BASE;

// Sequence stuff
const uint32_t INTERNAL = 0;
const uint32_t RISING_EDGE = 1;
const uint32_t CLOCK_DIV_RES_HIGH = 1;
const uint32_t CLOCK_DIV_RES_MED = 25;
const uint32_t CLOCK_DIV_RES_LOW = 250;
const uint32_t CLOCK_DIV_RES_VERY_LOW = 2500;
const uint32_t CLOCK_DIV_RES_VERY_VERY_LOW = 25000;
const uint32_t CLOCK_DIV_DEFAULT = CLOCK_DIV_RES_HIGH;
const uint32_t SEQUENCE_FLAG_END = 0;
const uint32_t ITERATIONS_MAX = 500000;
const uint32_t TRIGGER_SKIPS_MAX = 500;
const uint32_t CLOCK_DIVIDER_MAX = 50000;
const uint32_t PULSE_INSTRUCTION_OFFSET = 4;
const uint32_t CLOCK_INSTRUCTION_OFFSET = 1;
const uint32_t CLOCK_INSTRUCTION_MIN = PULSE_INSTRUCTIONS_MAX * PULSE_INSTRUCTION_OFFSET;
const uint32_t INTERNAL_CLOCK_IDS[CLOCKS_MAX] = {0, 1, 2};
const uint32_t INTERNAL_CLOCK_PINS[CLOCKS_MAX] = {16, 17, 18};
const uint32_t EXTERNAL_TRIGGER_PINS[TRIGGERS_MAX] = {13};
const double PULSE_UNITS_OFFSET_DEFAULT = 1e3; // microseconds
const double CLOCK_UNITS_OFFSET_DEFAULT = 1.0; // Hertz
