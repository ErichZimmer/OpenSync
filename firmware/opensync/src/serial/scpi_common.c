#include "scpi_common.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "scpi/scpi.h"

#include "status/sequencer_status.h"
#include "sequencer/sequencer_common.h"


const int32_t STATEFUL = -1;
const double DELAY_CLOCK_MIN = 320e-9; // 320 ns in seconds
const double DELAY_CLOCK_MAX = 16.0;   // 16 seconds
const double DELAY_PULSE_MIN = 44e-9;  // 44 ns in seconds
const double DELAY_PULSE_MAX = 8.0;    // 8 seconds
const uint32_t COUNTERS_MAX = 1000000000; // 1 billion max
const uint32_t BCOUNTER_MAX = 30000000; // 30 million max


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


// Make sure that delays below a nanosecond are clipped
double delay_round_nanoseconds(
    double delay_seconds
) {
    return round(delay_seconds * 1e9) / 1e9;
}


// Make sure a pulse or delay is a multiple of clock tick duration
double delay_round_clock_cycle(
    double delay,
    uint32_t clock_divider
) {
    if (!clock_divider) {
        clock_divider = 1;
    }

    double tick_ns = CLOCK_CYCLE_NS * (double) clock_divider;
    double cycles = round((delay * 1e9) / tick_ns);

    return (cycles * tick_ns) / 1e9;
}