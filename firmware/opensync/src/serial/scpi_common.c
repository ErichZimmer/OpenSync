#include "scpi_common.h"

#include <stdbool.h>
#include <stdint.h>

#include "scpi/scpi.h"

#include "status/sequencer_status.h"


const int32_t STATEFUL = -1;
const double DELAY_CLOCK_MIN = 200e-9; // 200 ns in seconds
const double DELAY_CLOCK_MAX = 16.0;   // 10 seconds
const double DELAY_PULSE_MIN = 44e-9;  // 44 ns in seconds
const double DELAY_PULSE_MAX = 8.0;    // 8 seconds
const uint32_t COUNTERS_MAX = 1000000000; // 1 billion max
const uint32_t BCOUNTER_MAX = 30000000; // 30 million max
const uint32_t DIVIDER_MAX = 65500;


bool is_running()
{
    uint32_t status_copy = sequencer_status_get();

    return !((status_copy == IDLE) || (status_copy == ABORTED));
}


// If the sequencer is running, push an error onto SCPI context and return true
bool SCPI_check_running_and_append_error(
    scpi_t* context
) {
    // If the system status is note (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return true;
    }

    return false;
}