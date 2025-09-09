#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define INSTRUCTIONS_MAX 64
#define TRIGGERS_MAX 2
#define CLOCKS_MAX 3


struct clock_config
{
    PIO pio;
    uint sm;
    uint clock_pin;
    uint trigger_pin;
    int dma_chan;
    uint32_t instructions[INSTRUCTIONS_MAX];
    uint32_t trigger_config[TRIGGERS_MAX];
    bool active;
};