#include "serial_int_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdio.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

#include "fast_serial.h"

#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "status/sequencer_status.h"


// Dump the clock config freerun instructions
void serial_print_clock_instructions(
    struct clock_config* config
) {
    for (uint32_t i = 0; i < CLOCK_INSTRUCTIONS_MAX; i = i + 2)
    {
        uint32_t inst_num = i / 2;
        uint32_t reps = config -> instructions[i];
        uint32_t delay = config -> instructions[i+1];

        fast_serial_printf("Clock instruction %i: reps=%i, delay=%i\r\n", inst_num, reps, delay);
    }
}


// Dump the clock config trigger instructions
void serial_print_clock_triggers(
    struct clock_config* config
) {
    for (uint32_t i = 0; i < CLOCK_TRIGGERS_MAX; i = i + 2)
    {
        uint32_t inst_num = i / 2;
        uint32_t skips = config -> trigger_config[i];
        uint32_t delay = config -> trigger_config[i+1];

        fast_serial_printf("Trigger instruction %i: skips=%i, delay=%i\r\n", inst_num, skips, delay);
    }
}


// Dump clock config settings
void serial_print_clock_configs(
    struct clock_config* config_array
)
{
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        fast_serial_printf("Clock config id: %i\r\n", i);
        fast_serial_printf("Clock config is active: %i\r\n", config_array[i].active);
        fast_serial_printf("Clock config clock type: %i\r\n", config_array[i].clock_type);
        fast_serial_printf("Clock config clock divider: %i\r\n", config_array[i].clock_divider);
        fast_serial_printf("Clock config sm: %i\r\n", config_array[i].sm);
        fast_serial_printf("Clock config dma channel: %i\r\n", config_array[i].dma_chan);
        fast_serial_printf("Clock config clock pin: %i\r\n", config_array[i].clock_pin);
        fast_serial_printf("Clock config trigger pin: %i\r\n", config_array[i].trigger_pin);
        serial_print_clock_instructions(
            &config_array[i]
        );
        serial_print_clock_triggers(
            &config_array[i]
        );

        fast_serial_printf("Clock config trigger reps: %i\r\n", config_array[i].trigger_reps);
    }
}


// Dump the pulse config freerun instructions
void serial_print_pulse_instructions(
    struct pulse_config* config
) {
    for (uint32_t i = 0; i < PULSE_INSTRUCTIONS_MAX; i = i + 2)
    {
        uint32_t inst_num = i / 2;
        uint32_t reps = config -> instructions[i];
        uint32_t delay = config -> instructions[i+1];

        fast_serial_printf("Pulse instruction %i: state=%b, delay=%i\r\n", inst_num, reps, delay);
    }
}


// Dump pulse config settings
void serial_print_pulse_configs(
    struct pulse_config* config_array
)
{
    for (uint32_t i = 0; i < CLOCKS_MAX; i++)
    {
        fast_serial_printf("Pulse config id: %i\r\n", i);
        fast_serial_printf("Pulse config is active: %i\r\n", config_array[i].active);
        fast_serial_printf("Pulse config clock divider: %i\r\n", config_array[i].clock_divider);
        fast_serial_printf("Pulse config sm: %i\r\n", config_array[i].sm);
        fast_serial_printf("Pulse config dma channel: %i\r\n", config_array[i].dma_chan);
        fast_serial_printf("Pulse config clock pin: %i\r\n", config_array[i].clock_pin);
        serial_print_pulse_instructions(
            &config_array[i]
        );
    }
}