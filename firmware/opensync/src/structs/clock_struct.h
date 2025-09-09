#pragma once 

#include <stdint.h>
#include "hardware/pio.h"


struct clock_config
{
    PIO pio;
    uint sm;
    uint clock_pin;
    uint trigger_pin;
    int dma_chan;
    uint32_t instructions[64];
    uint32_t trigger_config[2];
    uint32_t trigger_reps;
    bool active;
};