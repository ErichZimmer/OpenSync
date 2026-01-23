#include <stdbool.h>
#include <stdint.h>

#include "status/sequencer_status.h"
#include "scpi_common.h"


const uint64_t CLOCK_CYCLES_MAX = 4294967200; // 2^32 - 96
const uint64_t CLOCK_CYCLE_NANOS = 4; // Each clock cycle is 4 ns long


// Convert nanoseconds to cycles, return 1 if success, 0 is not
// TODO: round to nearest clock cycle using doubles instead of integer arithmetic
bool convert_nanos_to_cycles(
    uint64_t nanoseconds,
    uint32_t clock_divider,
    uint32_t* cycles
){
    uint64_t clock_cycles_raw = nanoseconds / ((uint64_t) (clock_divider) * CLOCK_CYCLE_NANOS);

    if (clock_cycles_raw > CLOCK_CYCLES_MAX)
    {
        return 0;
    }

    *cycles = (uint32_t) clock_cycles_raw;

    return 1;
}


bool is_running()
{
    uint32_t status_copy = sequencer_status_get();

    return !((status_copy == IDLE) || (status_copy == ABORTED));
}