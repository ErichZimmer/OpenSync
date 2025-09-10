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
#include "serial/serial_interface.h"

// #include "sequencer_pio_clock_freerun.pio.h"
// #include "sequencer_pio_clock_triggered.pio.h"
// #include "sequencer_pio_pulse_sequencer.pio.h"

// Third-party //
#include "fast_serial.h"

// Serial buffer//
#define SERIAL_BUFFER_SIZE 256
char serial_buf[SERIAL_BUFFER_SIZE];


int main()
{
    // Register sequencer_status mutex
    sequencer_status_register();

    // Ititialize serial interface
	fast_serial_init();

	// Set status to off
	sequencer_status_set(IDLE);

    while(1)
    {  
        uint32_t buf_len = fast_serial_read_until(
            serial_buf, 
            SERIAL_BUFFER_SIZE, 
            '\n'
        );

		uint32_t status_copy = sequencer_status_get();

		// Check command to see if it is at least four letters
		if(buf_len < 4)
        {
			fast_serial_printf("Invalid command: %s\r\n", serial_buf);
			continue;
		}

        // Get version
        if(strncmp(serial_buf, "vers", 4) == 0)
        {
			serial_print_version();
		}

        // Get system status
		else if(strncmp(serial_buf, "stat", 4) == 0)
        {
			serial_print_status();
		}

        // Set abortion
		else if(strncmp(serial_buf, "stop", 4) == 0)
        {
			if(status_copy == RUNNING || status_copy == ARMING){
				sequencer_status_set(ABORT_REQUESTED);
				fast_serial_printf("ok\r\n");
			}
			else {
				fast_serial_printf("Invalid request: Can only abort when status is 2 (RUNNING)\r\n");
			}
		}

                                        // Break point //
        // Commands beyond this level can only execute when state machines are not running
		else if (status_copy != ABORTED && status_copy != IDLE){
			fast_serial_printf("Invalid request: cannot execute command %s while running\r\n", serial_buf);
		}

        // Return system frequencies
		else if(strncmp(serial_buf, "freq", 4) == 0)
        {
			serial_print_freqs();
		}
    }
}