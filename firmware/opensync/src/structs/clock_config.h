#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define CLOCK_INSTRUCTIONS_MAX 16
// TODO: Rename this
#define CLOCK_TRIGGERS_MAX 2
#define CLOCKS_MAX 3
#define TRIGGERS_MAX 1

enum {
    CLOCK_FREERUN = 0,
    CLOCK_TRIGGERED,
    CLOCK_TRIGGERED_RISING,
    CLOCK_TRIGGERED_HIGH,
    CLOCK_TRIGGERED_FALLING,
    CLOCK_TRIGGERED_SNIFFER
};

struct clock_config
{
    PIO pio;
    uint sm;
    uint clock_pin;
    uint trigger_pin;
    uint32_t clock_type;
    int dma_chan;
    uint32_t __attribute__((aligned(CLOCK_INSTRUCTIONS_MAX * sizeof(uint32_t)))) instructions[CLOCK_INSTRUCTIONS_MAX];
    uint32_t __attribute__((aligned(CLOCK_TRIGGERS_MAX     * sizeof(uint32_t)))) trigger_config[CLOCK_TRIGGERS_MAX];
    uint32_t trigger_reps;
    uint clock_divider;
    double unit_offset;
    bool active;
    bool configured;
};