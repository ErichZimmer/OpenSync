#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "hardware/pio.h"

#include "structs/pulse_scpi_config.h"


extern const uint32_t ARM_SEQUENCER;

void core_1_init();

PIO get_pio_mapping(uint channel);

bool sequencer_configure(
    const struct pulse_scpi_config* scpi_interface
);

bool sequencer_dma_failed(int channel);

bool sequencer_run_ok();

bool sequencer_wait_for_event(
    PIO pio, 
    uint sm,
    uint offset,
    uint on_wait,
    uint off_wait
);

bool sequencer_wait_for_t0();

bool sequencer_wait_for_output(uint channel);

bool sequencer_start();

bool sequencer_wait();

void sequencer_free();