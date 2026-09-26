#pragma once

#include <stdint.h>


// Device status
extern const uint32_t SEQUENCER_DNDEBUG;
extern const uint32_t SEQUENCER_DEBUG;
extern const uint32_t SEQUENCER_DEBUG_ALL;

void debug_status_register(void);

void debug_status_set(uint32_t status_new);

uint32_t debug_status_get(void);

const char* debug_status_to_str(uint32_t status_copy);
