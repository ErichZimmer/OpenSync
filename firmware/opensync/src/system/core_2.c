#include "core_2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/dma.h"

#include "core_1.h"
#include "overclock/overclock.h"
#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "sequencer/sequencer_clock.h"
#include "serial/scpi-def.h"

#include "fast_serial.h"

// Serial buffer//
#define SERIAL_BUFFER_SIZE 64
char serial_buf[SERIAL_BUFFER_SIZE];


void core_2_init()
{
	// Set system clock speed
	overclock_system_set();

	// Register sequencer status mutexes
    sequencer_status_register();
	debug_status_register();

    // Initialize serial interface
	stdio_init_all();
	fast_serial_init();

	// Initialize device SCPI interface
    scpi_instrument_init();

	// Intialize sequencer cores
	multicore_launch_core1(core_1_init);
    multicore_fifo_pop_blocking();

	// Set system status to idle
	sequencer_status_set(IDLE);

    while(1)
    {  
        uint32_t buf_len = fast_serial_read_until(
            serial_buf, 
            SERIAL_BUFFER_SIZE, 
            '\n'
        );

		// Parse scpi command and hope for the best
		SCPI_Input(
            &scpi_context, 
            serial_buf, 
            strlen(serial_buf)
        );
    }
}