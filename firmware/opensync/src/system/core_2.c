#include "core_2.h"

// Serial buffer//
#define SERIAL_BUFFER_SIZE 256
char serial_buf[SERIAL_BUFFER_SIZE];

const uint32_t CMD_LEN = 4;
const uint32_t CMD_DETECTED = 0;

const uint32_t ARGS_MIN_SINGLE = 1;
const uint32_t ARGS_MIN_DOUBLE = 2;
const uint32_t ARGS_MIN_TRIPLE = 3;


void core_2_init()
{
	// Set system clock speed
	overlcok_system_set();

	// Register sequencer status mutexes
    sequencer_status_register();
	debug_status_register();

    // Initialize serial interface
	fast_serial_init();

	// Intialize sequencer cores
	multicore_launch_core1(core_1_init);
    multicore_fifo_pop_blocking();

	// Set status to idle
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
		if(buf_len < CMD_LEN)
        {
			fast_serial_printf("Invalid Request: Invalid command %s\r\n", serial_buf);
			continue;
		}

        // Get version
        if(strncmp(serial_buf, "vers", CMD_LEN) == CMD_DETECTED)
        {
			serial_print_version();
		}

		// Get device type
        if(strncmp(serial_buf, "type", CMD_LEN) == CMD_DETECTED)
        {
			serial_print_device_type();
		}

        // Get system status
		else if(strncmp(serial_buf, "stat", CMD_LEN) == CMD_DETECTED)
        {
			serial_print_status();
		}

        // Set abortion
		else if(strncmp(serial_buf, "stop", CMD_LEN) == CMD_DETECTED)
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
		else if(strncmp(serial_buf, "freq", CMD_LEN) == CMD_DETECTED)
        {
			serial_print_freqs();
		}

		// Set debug staus
		else if(strncmp(serial_buf, "dbug", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t debug_status_local = 0;

            int parsed = sscanf(serial_buf, "%*s %d", &debug_status_local);

            if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

            if (debug_status_local > 2)
            {
                fast_serial_printf("Invalid request: Debug status must be between 0 and 2\r\n");
				continue;
            }

			debug_status_set(debug_status_local);
			fast_serial_printf("ok\r\n");
		}

		// Set clock type
		else if(strncmp(serial_buf, "ctyp", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t requested_type = 0;

            int parsed = sscanf(serial_buf, "%*s %i", &requested_type);

            if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = sequencer_clock_type_set(requested_type);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set clock type\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set clock divider
		else if(strncmp(serial_buf, "cdiv", CMD_LEN) == CMD_DETECTED)
        {
			uint32_t clock_id = 0;
            uint32_t clock_divider_local = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &clock_id, &clock_divider_local);

            if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = clock_divider_set(
				clock_id,
				clock_divider_local
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set clock divider\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Activate clock ID (Clock ACTivate --> CACT)
		else if(strncmp(serial_buf, "cact", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t clock_id = 0;
			uint32_t clock_state = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &clock_id, &clock_state);

            if(parsed < ARGS_MIN_DOUBLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			if(clock_state > 1)
			{
				fast_serial_printf("Invalid request: Invalid clock state\r\n");
				continue;
			}

			bool success = clock_sequencer_state_set(
				clock_id,
				(bool) clock_state
			);

			if (!success){
				fast_serial_printf("Invalid request: Failed to set clock channel state\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Reset clock ID (Clock ReSeT --> CRST)
		else if(strncmp(serial_buf, "crst", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t clock_id = 0;

            int parsed = sscanf(serial_buf, "%*s %i", &clock_id);

            if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}


			bool success = clock_sequencer_state_reset(
				clock_id
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to reset clock channel state\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set clock external trigger pins
		else if(strncmp(serial_buf, "tset", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t clock_id = 0;
			uint32_t trigger_pin_id = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &clock_id, &trigger_pin_id);

            if(parsed < ARGS_MIN_DOUBLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = clock_pin_trigger_set(
				clock_id,
				trigger_pin_id
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set clock trigger pin\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set clock external trigger repitions
		else if(strncmp(serial_buf, "trep", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t clock_id = 0;
			uint32_t trigger_reps = 0;

            int parsed = sscanf(serial_buf, "%*s %d %d", &clock_id, &trigger_reps);

            if(parsed < ARGS_MIN_DOUBLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = clock_reps_trigger_set(
				clock_id,
				trigger_reps
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set clock trigger reps\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Load external trigger instructions (Trigger LoaD Instructions --> TLDI)
		else if(strncmp(serial_buf, "tldi", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t clock_id = 0;
			uint32_t trigger_skips = 0;
			uint32_t trigger_delay = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i %i", &clock_id, &trigger_skips, &trigger_delay);

            if(parsed < ARGS_MIN_TRIPLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			// This is not ideal, but there should only ever be two external trigger instructions and never more
			bool success = clock_trigger_instructions_load(
				clock_id,
				trigger_skips,
				trigger_delay
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to load clock trigger instructions\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Load clock instructions (Clock LoaD Instructions --> CLDI)
		else if(strncmp(serial_buf, "cldi", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t clock_id = 0;
			bool abort = false; // Abort is used to exit out in case invalid inputs are detected

			int parsed = sscanf(serial_buf, "%*s %i", &clock_id);

			if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			// Create instruction buffer
			uint32_t instruction_buffer[CLOCK_INSTRUCTIONS_MAX] = {0};

			// Fill instruction buffer
			uint32_t instruction_count = 0;
			uint32_t instruction_count_max = CLOCK_INSTRUCTIONS_MAX;

			while(true)
            {
				if (!(instruction_count < instruction_count_max))
				{
					fast_serial_printf("Invalid Request: Invalid instruction count (overflow)\r\n");
					abort = true;
					
					break;
				}

				uint32_t reps = 0, delay = 0, num_elements = 0;

				buf_len = fast_serial_read_until(serial_buf, SERIAL_BUFFER_SIZE, '\n');

				if(buf_len >= 4)
				{
					// Exit if requested
					if(strncmp(serial_buf, "exit", CMD_LEN) == CMD_DETECTED)
					{
						break;
					}
				}

                num_elements = sscanf(serial_buf, "%i %i", &reps, &delay);

				if (num_elements < ARGS_MIN_DOUBLE)
				{
					fast_serial_printf("Invalid Request: Invalid input\r\n");
					abort = true;

					break;
				}

                // Exit out of instruction loop
				if(strncmp(serial_buf, "exit", CMD_LEN) == CMD_DETECTED)
                {
					break;
				}

				if (reps > ITERATIONS_MAX)
				{
                    fast_serial_printf("Invalid Request: Invalid repition count: %i\r\n", reps);
					abort = true;

                    break;
                }

                instruction_buffer[instruction_count] = reps;
                instruction_count++;

                instruction_buffer[instruction_count] = delay;
                instruction_count++;

				// Everything is good so far, so return an ok
				fast_serial_printf("ok\r\n");
			}

			// Exit out if abort is true (which is the case for invalid inputs)
			if (abort)
			{
				continue;
			}

			// Load instructions into clock
			bool success = clock_instructions_load(
				clock_id,
				instruction_buffer
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to load clock instructions\r\n");
				continue;
			}

			fast_serial_printf("ok\r\n");
		}

		// Activate pulse ID (Pulse ACTivate --> PACT)
		else if(strncmp(serial_buf, "pact", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t pulse_id = 0;
			uint32_t pulse_state = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &pulse_id, &pulse_state);

            if(parsed < ARGS_MIN_DOUBLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			if(pulse_state > 1)
			{
				fast_serial_printf("Invalid request: Invalid clock state\r\n");
				continue;
			}

			bool success = pulse_sequencer_state_set(
				pulse_id,
				(bool) pulse_state
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set clock channel state\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set pulse clock divider
		else if(strncmp(serial_buf, "pdiv", CMD_LEN) == CMD_DETECTED)
        {
			uint32_t pulse_id = 0;
            uint32_t clock_divider_local = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &pulse_id, &clock_divider_local);

            if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = pulse_divider_set(
				pulse_id,
				clock_divider_local
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set pulse clock divider\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Reset pulse channel (Pulse ReSeT --> PRST)
		else if(strncmp(serial_buf, "prst", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t pulse_id = 0;

            int parsed = sscanf(serial_buf, "%*s %i", &pulse_id);

            if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = pulse_sequencer_state_reset(
				pulse_id
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to reset pulse channel state\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Set pulse sequencer internal clock pins
		else if(strncmp(serial_buf, "pset", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t pulse_id = 0;
			uint32_t clock_id = 0;

            int parsed = sscanf(serial_buf, "%*s %i %i", &pulse_id, &clock_id);

            if(parsed < ARGS_MIN_DOUBLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			bool success = pulse_pin_clock_set(
				pulse_id,
				clock_id
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to set internal clock pin\r\n");
				continue;
			}
			
			fast_serial_printf("ok\r\n");
		}

		// Load pulse instructions (Pulse LoaD Instructions --> PLDI)
		else if(strncmp(serial_buf, "pldi", CMD_LEN) == CMD_DETECTED)
        {
            uint32_t pulse_id = 0;
			bool abort = false; // Abort is used to exit out in case invalid inputs are detected

			int parsed = sscanf(serial_buf, "%*s %i", &pulse_id);

			if(parsed < ARGS_MIN_SINGLE)
			{
				fast_serial_printf("Invalid request: Invalid input\r\n");
				continue;
			}

			// Create instruction buffer
			uint32_t instruction_buffer[PULSE_INSTRUCTIONS_MAX] = {0};

			// Configure default buffer pattern
			/*
			Important Note:
			The pulse output instruction are broken up into two parts: the output
			state and the delay duration to the next instruction. For instance,
			every even instruction starting from zero (e.g., 0, 2, 4, 6, 8...) sets
			the output state. Only the first 12 bits are used to set the output state
			while the rest are discarded. The delay instructions are at every odd
			instruction index (e.g., 1, 3, 5, 7, 9...) and are used in a delay loop.
			The final two instruction at the end of the instruction buffer are used
			to set the output state to idle and a terminating flag.

			// DO NOT FORGET TO SET THE TERMINATING FLAGS (0) AND MAKE SURE ALL OTHER INSTRUCTIONS ARE NON-ZERO!!!!
			*/

			// Set all delay instructions to 1 (they can't be zero)
			for (int i = 1; i < PULSE_INSTRUCTIONS_MAX; i += 2)
			{
				instruction_buffer[i] = 1; // 1 cycle delay
			}

			// Make sure last two elements are zero
			instruction_buffer[PULSE_INSTRUCTIONS_MAX - 1] = SEQUENCE_FLAG_END;
			instruction_buffer[PULSE_INSTRUCTIONS_MAX - 2] = SEQUENCE_FLAG_END;

			// Fill instruction buffer
			uint32_t instruction_flag_count = 2;
			uint32_t instruction_count = 0;
			uint32_t instruction_count_max = PULSE_INSTRUCTIONS_MAX - instruction_flag_count;

			while(true)
            {
				if (!(instruction_count < instruction_count_max))
				{
					fast_serial_printf("Invalid Request: Invalid instruction count (overflow)\r\n");
					abort = true;
					
					break;
				}

				uint32_t output = 0, delay = 0, num_elements = 0;

				buf_len = fast_serial_read_until(serial_buf, SERIAL_BUFFER_SIZE, '\n');

				if(buf_len >= CMD_LEN)
				{
					// Exit if requested
					if(strncmp(serial_buf, "exit", CMD_LEN) == CMD_DETECTED)
					{
						break;
					}
				}

                num_elements = sscanf(serial_buf, "%i %i", &output, &delay);

				if (num_elements < ARGS_MIN_DOUBLE)
				{
					fast_serial_printf("Invalid Request: Invalid input\r\n");
					abort = true;
					
					break;
				}

                // Self explanatory
				if(strncmp(serial_buf, "exit", CMD_LEN) == CMD_DETECTED)
                {
					break;
				}

                // Validate output
                if(output & ~OUT_MASK)
                {
                    fast_serial_printf("Invalid Request: Invalid output state: %i\r\n", output);
					abort = true;

                    break;
                }

				// Validate delay cycles
                if(delay < (PULSE_INSTRUCTION_OFFSET + 1))
                {
                    fast_serial_printf("Invalid Request: Invalid delay (cycles): %i\r\n", delay);
					abort = true;

                    break;
                }

                instruction_buffer[instruction_count] = output;
                instruction_count++;

                instruction_buffer[instruction_count] = delay;
                instruction_count++;

				// Everything is good so far, so return an ok
				fast_serial_printf("ok\r\n");
			}

			// Make sure the final instructions are zero (again)
			instruction_buffer[PULSE_INSTRUCTIONS_MAX - 1] = SEQUENCE_FLAG_END;
			instruction_buffer[PULSE_INSTRUCTIONS_MAX - 2] = SEQUENCE_FLAG_END;

			// exit out if abort is true (set by invalid input)
			if (abort)
			{
				continue;
			}

			// Load instructions into clock
			bool success = pulse_instructions_load(
				pulse_id,
				instruction_buffer
			);

			if (!success)
			{
				fast_serial_printf("Invalid request: Failed to load pulse instructions\r\n");
				continue;
			}

			fast_serial_printf("ok\r\n");
		}

		// Fire sequencer with current settings
		else if(strncmp(serial_buf, "fire", CMD_LEN) == CMD_DETECTED)
        {
			multicore_fifo_push_blocking(ARM_SEQUENCER);
			fast_serial_printf("ok\r\n");
		}

		else
		{
			fast_serial_printf("Invalid Request: Command not found\r\n");
		}
    }
}