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

void sequencer_set_status(uint32_t status_new);

int sequencer_get_status(void);

const char* sequencer_get_status_str(uint32_t status_copy);