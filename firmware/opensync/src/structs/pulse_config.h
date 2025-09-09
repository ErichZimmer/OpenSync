#pragma once 

#include <stdint.h>
#include "hardware/pio.h"


struct pulse_config
{
    PIO pio;
    uint sm;
    uint out_pins_base;
    uint out_pins_count;
    uint clock_pin;
    int dma_chan;
    uint32_t instructions[64];
    bool active;
};