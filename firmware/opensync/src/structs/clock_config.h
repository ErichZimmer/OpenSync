#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define CLOCK_INSTRUCTIONS_MAX 64
#define CLOCK_TRIGGERS_MAX 2
#define CLOCKS_MAX 3

enum {
    CLOCK_FREERUN = 0,
    CLOCK_TRIGGERED
};

struct clock_config
{
    PIO pio;
    uint sm;
    uint clock_pin;
    uint trigger_pin;
    uint32_t clock_type;
    int dma_chan;
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX];
    uint32_t trigger_config[CLOCK_TRIGGERS_MAX];
    uint32_t trigger_reps;
    uint clock_divider;
    bool active;
    bool configured;
};