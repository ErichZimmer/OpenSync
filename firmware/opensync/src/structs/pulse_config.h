#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define PULSE_INSTRUCTIONS_MAX 32
#define PULSE_INSTRUCTIONS_OUTPUT_TERM PULSE_INSTRUCTIONS_MAX - 2
#define PULSE_INSTRUCTIONS_DELAY_TERM PULSE_INSTRUCTIONS_MAX - 1
#define PULSE_ITERATIONS_MAX 500000


struct pulse_config
{
    PIO pio;
    uint sm;
    uint out_pins_base;
    uint out_pins_count;
    uint clock_pin;
    int dma_chan;
    uint32_t __attribute__((aligned(PULSE_INSTRUCTIONS_MAX * sizeof(uint32_t)))) instructions[PULSE_INSTRUCTIONS_MAX];
    uint clock_divider;
    double unit_offset;
    bool active;
    bool configured;
};