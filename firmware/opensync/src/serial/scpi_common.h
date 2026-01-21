#pragma once

#include <stdbool.h>

bool convert_nanos_to_cycles(
    uint64_t nanoseconds,
    uint32_t clock_divider,
    uint32_t* cycles
);

bool is_running();