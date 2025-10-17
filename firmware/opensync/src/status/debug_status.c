#include "debug_status.h"

#include <stdint.h>
#include "pico/mutex.h"
 

// Device status
const uint32_t SEQUENCER_DNDEBUG = 0;
const uint32_t SEQUENCER_DEBUG = 1;
const uint32_t SEQUENCER_DEBUG_ALL = 2;


// Mutex for status
static mutex_t debug_mutex_status;
uint32_t debug_status = SEQUENCER_DNDEBUG;

// This must be called in main before anything else.
void debug_status_register()
{
    mutex_init(&debug_mutex_status);
}


void debug_status_set(uint32_t status_new)
{
	mutex_enter_blocking(&debug_mutex_status);
	debug_status = status_new;
	mutex_exit(&debug_mutex_status);
}


uint32_t debug_status_get()
{
	mutex_enter_blocking(&debug_mutex_status);
	uint32_t status_copy = debug_status;
	mutex_exit(&debug_mutex_status);

	return status_copy;
}


const char* debug_status_to_str(uint32_t status_copy)
{
    switch (status_copy)
    {
        case SEQUENCER_DNDEBUG:
            return "DO NO DEBUG";
        case SEQUENCER_DEBUG:
            return "DEBUG 1";
        case SEQUENCER_DEBUG_ALL:
            return "DEBUG 2";
        default:
            return "Not Supported";
    }
}