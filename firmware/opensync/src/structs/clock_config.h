#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define CLOCK_INSTRUCTIONS_MAX 64
#define CLOCK_TRIGGERS_MAX 2
#define CLOCKS_MAX 3


struct clock_config
{
    PIO pio;
    uint sm;
    uint clock_pin;
    uint trigger_pin;
    int dma_chan;
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX];
    uint32_t trigger_config[CLOCK_TRIGGERS_MAX];
    uint32_t trigger_reps;
    uint32_t clock_divider;
    bool active;
    bool configured;
};