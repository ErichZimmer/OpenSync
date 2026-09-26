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

#include "system/core_1.h"
#include "overclock/overclock.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "serial/scpi-def.h"
#include "serial/scpi_sequencer.h"

#include "fast_serial.h"

// Serial buffer//
#define SERIAL_BUFFER_SIZE 128
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

    // Clear any data in SCPI pulse sequencer interface
    pulse_channels_clear();

    // Memory initialization and defualt selection will go here.
    

	// Intialize sequencer cores
	multicore_launch_core1(core_1_init);

    const uint32_t core_1_state = multicore_fifo_pop_blocking();

    if (!core_1_state)
    {
        SCPI_ErrorPush(
            &scpi_context, 
            SCPI_ERROR_SYSTEM_ERROR
        );
        // Core 1 has already set PROGRAM_FAILURE.
    }
    else
    {
        sequencer_status_set(IDLE);
    }

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