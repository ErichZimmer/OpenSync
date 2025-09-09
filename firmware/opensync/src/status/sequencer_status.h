#pragma once

#include "pico/multicore.h"
#include <stdint.h>

// Device status
extern const uint32_t IDLE;
extern const uint32_t ARMING;
extern const uint32_t RUNNING;
extern const uint32_t ABORT_REQUESTED\;
extern const uint32_t ABORTING;
extern const uint32_t ABORTED;
extern const uint32_t DISARMING;

extern const uint32_t STAND_BY;
extern const uint32_t SEQUENCE_TRIGGER_START;

extern uin32_t sequencer_status;

void sequencer_status_register(void);

void sequencer_status_set(uint32_t status_new);

int sequencer_status_get(void);

const char* sequencer_status_to_str(uint32_t status_copy);