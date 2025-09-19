#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdio.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

// Local
#include "version/opensync_version.h"
#include "structs/clock_config.h"
#include "structs/pulse_config.h"

// Third-party //
#include "fast_serial.h"


void serial_print_version();

void serial_print_status();

void serial_print_clock_configs(
    struct clock_config* config_array
);

void serial_print_pulse_configs(
    struct pulse_config* config_array
);

void serial_print_freqs();