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

// Third-party //
#include "fast_serial.h"


void serial_print_version();

void serial_print_status();

void serial_print_freqs();