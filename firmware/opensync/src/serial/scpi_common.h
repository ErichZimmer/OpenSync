#pragma once

#include <stdbool.h>


#define STATUS_ON 1
#define STATUS_OFF 0

extern const int32_t STATEFUL;

bool convert_nanos_to_cycles(
    uint64_t nanoseconds,
    uint32_t clock_divider,
    uint32_t* cycles
);

bool is_running();