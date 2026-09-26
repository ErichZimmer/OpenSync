#include "tusb.h"
#include "pico/unique_id.h"

#include <stdarg.h>

#include "fast_serial.h"

/*
  Serial functions

  These are mostly thin wrappers around TinyUSB functions;
  they are provided to simplify the API.
 */

// Read bytes (blocks until buffer_size is reached)
uint32_t fast_serial_read(const char * buffer, uint32_t buffer_size){
	uint32_t buffer_idx = 0;
	while(buffer_idx < buffer_size){
		uint32_t buffer_avail = buffer_size - buffer_idx;
		uint32_t read_avail = fast_serial_read_available();

		if(read_avail > 0){
			if(buffer_avail > read_avail){
				buffer_avail = read_avail;
			}

			buffer_idx += fast_serial_read_atomic(buffer + buffer_idx, buffer_avail);
		}

		fast_serial_task();
	}
	return buffer_size;
}

// Read bytes until terminator reached (blocks until terminator or buffer_size is reached)
uint32_t fast_serial_read_until(char * buffer, uint32_t buffer_size, char until){
	uint32_t buffer_idx = 0;
	while(buffer_idx < buffer_size - 1){
		while(fast_serial_read_available() > 0){
			int32_t next_char = tud_cdc_read_char();

			buffer[buffer_idx] = next_char;
			buffer_idx++;
			if(next_char == until){
				break;
			}
		}

		if(buffer_idx > 0 && buffer[buffer_idx-1] == until){
			break;
		}
		fast_serial_task();
	}
	buffer[buffer_idx] = '\0'; // Null terminate string
	return buffer_idx;
}

// Write bytes (without flushing, so limited to 64 bytes)
uint32_t fast_serial_write(const char * buffer, uint32_t buffer_size){
	uint32_t buffer_idx = 0;
	while(buffer_idx < buffer_size){
		uint32_t write_avail = fast_serial_write_available();

		if(write_avail > 0){
			if(buffer_size - buffer_idx < write_avail){
				write_avail = buffer_size - buffer_idx;
			}

			buffer_idx += fast_serial_write_atomic(buffer + buffer_idx, write_avail);
		}
		fast_serial_task();
		fast_serial_write_flush();
	}
	return buffer_size;
}

int fast_serial_printf(const char * format, ...){
	va_list va;
	va_start(va, format);
	char printf_buffer[128];
	int ret = vsnprintf(printf_buffer, 128, format, va);
	va_end(va);
	if(ret <= 0){
		return ret;
	}
	return fast_serial_write(printf_buffer, strnlen(printf_buffer, 128));
}