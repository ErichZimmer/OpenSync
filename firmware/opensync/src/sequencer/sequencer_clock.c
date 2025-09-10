#include "sequencer_clock.h"

// Used for user input validation
const uint32_t OUT_MASK = ((1u << OUTPUT_PIN_COUNT) - 1) << OUTPUT_PIN_BASE;

// Sequence stuff
const uint32_t ITERATIONS_MAX = 500000;
const uint32_t CLOCK_DIVIDER_MAX = 65000;
const uint32_t INTERNAL_CLOCK_PINS[3] = {16, 17, 18};
const uint32_t EXTERNAL_TRIGGER_PINS[3] = {13, 14, 15};


void sequencer_clocks_init(
    struct clock_config* config_array,
    PIO clock_pio
) {
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        config_array[i].pio = clock_pio;
        config_array[i].sm = i;
        config_array[i].clock_pin = INTERNAL_CLOCK_PINS[i];
        config_array[i].trigger_pin = EXTERNAL_TRIGGER_PINS[i];
        config_array[i].active = false;
    }
}


bool sequencer_clocks_configure(
    struct clock_config* config_array,
    uint32_t* clock_pins,
    uint32_t* trigger_pins,
    uint32_t num_clocks
) {
    if (num_clocks >= CLOCKS_MAX)
    {
        return 0;
    }

    for (uint32_t i = 0; i < num_clocks; i++)
    {
        config_array[i].clock_pin = clock_pins[i];
        config_array[i].trigger_pin = trigger_pins[i];
        config_array[i].active = true;
    }

    return 1;
}


bool sequencer_insert_instructions_internal(
    struct clock_config* config_array,
    uint32_t (*instructions)[CLOCK_INSTRUCTIONS_MAX],
    uint32_t num_clocks
) {
    if (num_clocks >= CLOCKS_MAX)
    {
        return 0;
    }
    
    for (uint32_t i = 0; i < num_clocks; i++)
    {
        for (uint32_t j = 0; j < CLOCK_INSTRUCTIONS_MAX; j++)
        {
            config_array[i].instructions[j] = instructions[i][j];
        }
    }

    return 1;
}


bool sequencer_insert_instructions_triggered(
    struct clock_config* config_array,
    uint32_t (*instructions)[CLOCK_TRIGGERS_MAX],
    uint32_t num_clocks
) {
    if (num_clocks >= CLOCKS_MAX)
    {
        return 0;
    }
    
    for (uint32_t i = 0; i < num_clocks; i++)
    {
        for (uint32_t j = 0; j < CLOCK_TRIGGERS_MAX; j++)
        {
            config_array[i].trigger_config[j] = instructions[i][j];
        }
    }

    return 1;
}