#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define PULSE_INSTRUCTIONS_MAX 64


struct pulse_config
{
    PIO pio;
    uint sm;
    uint out_pins_base;
    uint out_pins_count;
    uint clock_pin;
    int dma_chan;
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX];
    bool active;
};