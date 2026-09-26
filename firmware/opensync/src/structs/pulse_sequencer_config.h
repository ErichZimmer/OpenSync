#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware/pio.h"

#define PULSE_SEQUENCER_SEGMENTS 6u
#define PULSE_SEQUENCER_WORDS 8u
#define PULSE_SEQUENCER_RING_BITS 5u
#define PULSE_SEQUENCER_DELAY_MAX 0x7fffffffu
#define PULSE_SEQUENCER_DMA_MAX 0x0fffffffu

// One runtime object per output channel, stored in core_1's static array.
// The SCPI settings are a separate struct pulse_scpi_config.
struct pulse_sequencer_config
{
    PIO pio;
    uint sm;
    uint program_offset;       // core_1 loads one shared program per PIO block
    int dma_chan;

    uint pin_out;
    uint pin_trigger;          // Normally T0 GPIO 2
    uint pin_gate;
    uint clock_divider;
    uint32_t wcounter;
    uint32_t bcounter;
    uint32_t pcounter;
    uint32_t ocounter;
    uint32_t gate_type;        // GET1: 0=T0 skip, 1=output inhibit

    // Six SCPI segments, one LOW/D=1 padding word, then one zero terminator.
    uint32_t __attribute__((aligned(32))) instructions[PULSE_SEQUENCER_WORDS];

    bool gate_enabled;         // core_1 masks channel gates unless T0 mode=CHANNEL
    bool active;
    bool configured;
    bool sm_claimed;
};