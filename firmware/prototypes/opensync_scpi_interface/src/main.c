#include <stdio.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "serial_interface/scpi-def.h"
#include "fast_serial.h"

#define SERIAL_BUFFER_SIZE 64

char serial_input_buffer[SERIAL_BUFFER_SIZE];

int main()
{
    stdio_init_all();

    // Initialize serial interface
	fast_serial_init();

    //Initialize SCPI interface
    scpi_instrument_init();

    while(1)
    {  
        uint32_t buf_len = fast_serial_read_until(
            serial_input_buffer, 
            SERIAL_BUFFER_SIZE, 
            '\n'
        );

        SCPI_Input(
            &scpi_context, 
            serial_input_buffer, 
            strlen(serial_input_buffer)
        );

    }

    return 0;
}