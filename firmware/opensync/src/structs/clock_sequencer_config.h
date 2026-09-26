#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware/pio.h"

// Pico 2 / RP2350 runtime state. core_1 owns one static object per clock.
// SCPI settings remain in the separate struct pulse_scpi_config.
#define CLOCK_SEQUENCER_DMA_MAX 0x0fffffffu
#define CLOCK_SEQUENCER_MIN_DELAY 9u

struct clock_sequencer_config
{
    PIO pio;
    uint sm;
    uint program_offset;       // Loaded once and supplied by core_1
    int dma_chan;              // -1 for unclaimed; always -1 for T0

    uint32_t bcounter;         // Internal: total acquisitions, NOT ticks per trigger
    uint32_t pcounter;         // Exact ON count
    uint32_t ocounter;         // Exact OFF count
    uint32_t delay;            // Internal: encoded D; tick period = D + 8 SM cycles
    uint32_t __attribute__((aligned(4))) instructions; // One-word DMA read ring

    uint pin_out;              // Internal GPIO 3, or the single T0 GPIO 2
    uint pin_trigger;          // External trigger, or internal-clock input for T0
    uint pin_gate;             // Active-HIGH inhibit; board supplies LOW if unused
    uint clock_divider;        // Integer SM divider, 1..65535

    bool active;               // Caller selection flag, like pulse_config.active
    bool configured;
    bool sm_claimed;           // Tracks ownership during partial setup/cleanup
};
