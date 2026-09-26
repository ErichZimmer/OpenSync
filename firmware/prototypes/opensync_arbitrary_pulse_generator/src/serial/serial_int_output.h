#pragma once

#include "structs/clock_config.h"
#include "structs/pulse_config.h"


void serial_print_version();

void serial_print_device_type();

void serial_print_status();

void serial_print_clock_configs(
    struct clock_config* config_array
);

void serial_print_pulse_configs(
    struct pulse_config* config_array
);

void serial_print_freqs();