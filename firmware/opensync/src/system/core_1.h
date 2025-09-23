#pragma once

#include <stdint.h>
#include "pico/multicore.h"

#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer/sequencer_clock.h"
#include "sequencer/sequencer_output.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "serial/serial_int_output.h"

void core_1_init();