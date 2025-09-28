#include "core_2.h"
#include "hardware/gpio.h"

// Serial buffer//
#define SERIAL_BUFFER_SIZE 256
char serial_buf[SERIAL_BUFFER_SIZE];


void core_2_init()
{
    // Register sequencer_status mutex
    sequencer_status_register();
	debug_status_register();

    // Initialize serial interface
	fast_serial_init();

	// Intialize sequencer core
	multicore_launch_core1(core_1_init);
    multicore_fifo_pop_blocking();

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

		// Set debug staus
		else if(strncmp(serial_buf, "dbug", 4) == 0)
        {
            uint32_t debug_status_local = 0;

            int parsed = sscanf(serial_buf, "%*s %d", &debug_status_local);

            if(parsed < 1){
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

            if ((debug_status_local < 0) || (debug_status_local > 2))
            {
                fast_serial_printf("Invalid request: Debug status must be between 0 and 2\r\n");
				continue;
            }

			debug_status_set(debug_status_local);
			fast_serial_printf("ok\r\n");
		}

		// Set clock divider
		else if(strncmp(serial_buf, "cdiv", 4) == 0)
        {
            uint32_t clock_divider_local = 0;

            int parsed = sscanf(serial_buf, "%*s %i", &clock_divider_local);

            if(parsed < 1){
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = clock_divider_set(clock_divider_local);

			if (!success){
				fast_serial_printf("Invalid request: Failed to set clock divider\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set clock external trigger pins
		else if(strncmp(serial_buf, "tset", 4) == 0)
        {
            uint32_t clock_id = 0;
			uint32_t trigger_pin_id = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &clock_id, &trigger_pin_id);

            if(parsed < 2){
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = clock_pin_trigger_set(
				clock_id,
				trigger_pin_id
			);

			if (!success){
				fast_serial_printf("Invalid request: Failed to set clock trigger pin\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set clock external trigger repitions
		else if(strncmp(serial_buf, "trep", 4) == 0)
        {
            uint32_t clock_id = 0;
			uint32_t trigger_reps = 0;

            int parsed = sscanf(serial_buf, "%*s %d %d", &clock_id, &trigger_reps);

            if(parsed < 2){
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = clock_reps_trigger_set(
				clock_id,
				trigger_reps
			);

			if (!success){
				fast_serial_printf("Invalid request: Failed to set clock trigger reps\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Load external trigger instructions (Trigger LoaD Instructions --> TLDI)
		else if(strncmp(serial_buf, "tldi", 4) == 0)
        {
            uint32_t clock_id = 0;
			uint32_t trigger_skips = 0;
			uint32_t trigger_delay = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i %i", &clock_id, &trigger_skips, &trigger_delay);

            if(parsed < 3){
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			// This is not ideal, but there should only ever be two external trigger instructions and never more
			bool success = clock_trigger_instructions_load(
				clock_id,
				trigger_skips,
				trigger_delay
			);

			if (!success){
				fast_serial_printf("Invalid request: Failed to load clock trigger instructions\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set pulse sequencer internal clock pins
		else if(strncmp(serial_buf, "pset", 4) == 0)
        {
            uint32_t pulse_id = 0;
			uint clock_id = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &pulse_id, &clock_id);

            if(parsed < 2){
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = pulse_pin_clock_set(
				pulse_id,
				clock_id
			);

			if (!success){
				fast_serial_printf("Invalid request: Failed to set internal clock pin\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Fire sequencer with current settings
		else if(strncmp(serial_buf, "fire", 4) == 0)
        {
			multicore_fifo_push_blocking(1);
			fast_serial_printf("ok\r\n");
		}

		else
		{
			fast_serial_printf("Invalid request\r\n");
		}
    }
}