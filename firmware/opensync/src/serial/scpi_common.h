#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "scpi/scpi.h"


#define STATUS_ON 1
#define STATUS_OFF 0

enum {
    NANOSECOND = 0,
    MICROSECOND,
    MILLISECOND,
    SECOND,
    MINUTE,
    HOUR
};

extern const int32_t STATEFUL;
extern const double OFFSET_NANOSECOND;
extern const double OFFSET_MICROSECOND;
extern const double OFFSET_MILLISECOND;
extern const double OFFSET_SECOND;
extern const double OFFSET_MINUTE;
extern const double OFFSET_HOUR;

bool convert_nanos_to_cycles(
    uint64_t nanoseconds,
    uint32_t clock_divider,
    uint32_t* cycles
);

bool is_running();

bool SCPI_check_running_and_append_error(
    scpi_t* context
);