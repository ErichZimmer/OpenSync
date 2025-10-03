#pragma once

#include <stdint.h>
#include "pico/multicore.h"

#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer/sequencer_common.h"
#include "sequencer/sequencer_clock.h"
#include "sequencer/sequencer_output.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "serial/serial_int_output.h"

extern const uint32_t ARM_SEQUENCER;

void core_1_init();

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

void sequencer_sm_active_free();

bool sequencer_pulse_validate(
    struct pulse_config* config
);

bool sequencer_clock_type_set(
    uint32_t requested_type
);

bool clock_id_validate(
    uint32_t clock_id
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

bool clock_pin_trigger_set(
    uint32_t clock_id,
    uint32_t trigger_pin
);

bool clock_reps_trigger_set(
    uint32_t clock_id,
    uint32_t trigger_reps
);

bool clock_trigger_instructions_load(
    uint32_t clock_id,
    uint32_t trigger_skips,
    uint32_t trigger_delay
);

bool clock_instructions_load(
    uint32_t clock_id,
    uint32_t instructions[CLOCK_INSTRUCTIONS_MAX]
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

bool pulse_instructions_load(
    uint32_t pulse_id,
    uint32_t instructions[PULSE_INSTRUCTIONS_MAX]
);

bool pulse_sequencer_state_reset(
    uint32_t pulse_id
);