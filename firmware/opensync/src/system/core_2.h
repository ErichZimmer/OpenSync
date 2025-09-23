#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/bootrom.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/dma.h"

// Local //
#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer/sequencer_clock.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "serial/serial_int_output.h"
#include "core_1.h"

// Third-party //
#include "fast_serial.h"

void core_2_init();