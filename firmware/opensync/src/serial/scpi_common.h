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
extern const uint32_t COUNTERS_MAX;
extern const uint32_t BCOUNTER_MAX;
extern const uint32_t DIVIDER_MAX;
extern const double DELAY_CLOCK_MIN;
extern const double DELAY_CLOCK_MAX;
extern const double DELAY_PULSE_MIN;
extern const double DELAY_PULSE_MAX;

bool is_running();

bool SCPI_check_running_and_append_error(
    scpi_t* context
);