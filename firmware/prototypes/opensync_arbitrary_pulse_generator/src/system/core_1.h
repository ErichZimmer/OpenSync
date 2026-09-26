#pragma once

#include <stdint.h>

#include "structs/clock_config.h"
#include "structs/pulse_config.h"


extern const uint32_t ARM_SEQUENCER;

void core_1_init();

struct clock_config* sequencer_clock_config_get();

struct pulse_config* sequencer_pulse_config_get();

void debug_message_print(
    uint32_t debug_status_local,
    char* message
);

void debug_message_print_i(
    uint32_t debug_status_local,
    char* message,
    int num
);

void sequencer_clock_sm_config_active();

void sequencer_output_sm_config_active();

uint sequencer_clock_sm_mask_get();

uint sequencer_output_sm_mask_get();

void sequencer_clock_sm_stall();

void sequencer_sm_active_free();

bool sequencer_pulse_conflict_check();

bool sequencer_pulse_validate(
    struct pulse_config* config
);

bool sequencer_clock_mode_set(
    uint32_t clock_id,
    uint32_t requested_mode
);

bool sequencer_trigger_mode_set(
    uint32_t clock_id,
    uint32_t requested_mode
);

bool sequencer_trigger_edge_set(
    uint32_t clock_id,
    uint32_t requested_edge
);

bool sequencer_trigger_level_set(
    uint32_t clock_id,
    uint32_t requested_level
);

bool clock_id_validate(
    uint32_t clock_id
);

bool trigger_id_validate(
    uint32_t pulse_id
);

bool pulse_id_validate(
    uint32_t pulse_id
);

bool clock_divider_set(
    uint32_t clock_id,
    uint32_t clock_divider_copy
);

bool pulse_divider_set(
    uint32_t pulse_id,
    uint32_t clock_divider_copy
);

bool clock_sequencer_state_set(
    uint32_t clock_id,
    bool clock_state
);

bool trigger_pin_set(
    uint32_t clock_id,
    uint32_t trigger_pin_id
);

bool trigger_count_set(
    uint32_t clock_id,
    uint32_t trigger_reps
);

bool trigger_skip_set(
    uint32_t clock_id,
    uint32_t trigger_skips
);

bool trigger_delay_set(
    uint32_t clock_id,
    uint32_t trigger_delay
);

bool clock_instructions_load(
    uint32_t clock_id,
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX]
);

bool clock_unit_offset_set(
    uint32_t clock_id,
    double units_offset
);

bool clock_trigger_unit_offset_set(
    uint32_t clock_id,
    double units_offset
);


bool clock_sequencer_state_reset(
    uint32_t clock_id
);

bool pulse_sequencer_state_set(
    uint32_t pulse_id,
    bool pulse_state
);

bool pulse_pin_clock_set(
    uint32_t pulse_id,
    uint32_t clock_id
);

bool pulse_unit_offset_set(
    uint32_t pulse_id,
    double units_offset
);

bool pulse_instructions_load(
    uint32_t pulse_id,
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX]
);

bool pulse_sequencer_state_reset(
    uint32_t pulse_id
);