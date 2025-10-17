#include "sequencer_status.h"

#include <stdint.h>
#include "pico/mutex.h"


// Device status
const uint32_t IDLE = 0;
const uint32_t ARMING = 1;
const uint32_t RUNNING = 2;
const uint32_t ABORT_REQUESTED = 3;
const uint32_t ABORTING = 4;
const uint32_t ABORTED = 5;
const uint32_t DISARMING = 6;

const uint32_t STAND_BY = 0;

// Mutex for status
static mutex_t sequencer_mutex_status;
uint32_t sequencer_status = IDLE;

// This must be called in main before anything else.
void sequencer_status_register()
{
    mutex_init(&sequencer_mutex_status);
}


void sequencer_status_set(uint32_t status_new)
{
	mutex_enter_blocking(&sequencer_mutex_status);
	sequencer_status = status_new;
	mutex_exit(&sequencer_mutex_status);
}


uint32_t sequencer_status_get()
{
	mutex_enter_blocking(&sequencer_mutex_status);
	uint32_t status_copy = sequencer_status;
	mutex_exit(&sequencer_mutex_status);

	return status_copy;
}


const char* sequencer_status_to_str(uint32_t status_copy)
{
    switch (status_copy)
    {
        case IDLE:
            return "IDLE";
        case ARMING:
            return "ARMING";
        case RUNNING:
            return "RUNNING";
        case ABORT_REQUESTED:
            return "ABORT_REQUESTED";
        case ABORTING:
            return "ABORTING"; 
        case ABORTED:
            return "ABORTED"; 
        case DISARMING:
            return "DISARMING"; 
        default:
            return "Not Supported";
    }
}