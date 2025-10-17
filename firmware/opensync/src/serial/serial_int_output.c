#include "serial_int_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdio.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

#include "fast_serial.h"

#include "version/opensync_version.h"
#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "status/sequencer_status.h"


// Dump firmware version
void serial_print_version()
{
    fast_serial_printf("Version: %s\r\n", OPENSYNC_VERSION);
}

// Dump device type
void serial_print_device_type()
{
    fast_serial_printf("Device: OpenSync\r\n");
}

// Dump system status
void serial_print_status()
{
    uint32_t status_copy = sequencer_status_get();

    fast_serial_printf("Run status: %s\r\n", sequencer_status_to_str(status_copy));
}


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
        uint32_t skips = config -> instructions[i];
        uint32_t delay = config -> instructions[i+1];

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
        fast_serial_printf("Clock config ID: %i\r\n", i);
        fast_serial_printf("Clock config sm: %i\r\n", config_array[i].sm);
        fast_serial_printf("Clock config DMA channel: %i\r\n", config_array[i].dma_chan);
        fast_serial_printf("Clock config clock divider: %i\r\n", config_array[i].clock_divider);
        fast_serial_printf("Clock config clock pin: %i\r\n", config_array[i].clock_pin);
        fast_serial_printf("Clock config trigger pin: %i\r\n", config_array[i].trigger_pin);
        serial_print_clock_instructions(
            &config_array[i]
        );
        serial_print_clock_triggers(
            &config_array[i]
        );

        fast_serial_printf("Clock config trigger reps: %i\r\n", config_array[i].trigger_reps);

        fast_serial_printf("Clock config is active: %i\r\n", config_array[i].active);
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
        fast_serial_printf("Pulse config ID: %i\r\n", i);
        fast_serial_printf("Pulse config sm: %i\r\n", config_array[i].sm);
        fast_serial_printf("Pulse config DMA channel: %i\r\n", config_array[i].dma_chan);
        fast_serial_printf("Pulse config clock divider: %i\r\n", config_array[i].clock_divider);
        fast_serial_printf("Pulse config pulse pin: %i\r\n", config_array[i].clock_pin);
        serial_print_pulse_instructions(
            &config_array[i]
        );
        fast_serial_printf("Pulse config is active: %i\r\n", config_array[i].active);
    }
}


// Report system frequencies
// See https://github.com/raspberrypi/pico-examples/blob/master/clocks/hello_48MHz/hello_48MHz.c
// Liscensed under BSD-3-Clause License
void serial_print_freqs()
{
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);

    #ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
        uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
    #endif
    
    fast_serial_printf("pll_sys = %dkHz\r\n", f_pll_sys);
    fast_serial_printf("pll_usb = %dkHz\r\n", f_pll_usb);
    fast_serial_printf("rosc = %dkHz\r\n", f_rosc);
    fast_serial_printf("clk_sys = %dkHz\r\n", f_clk_sys);
    fast_serial_printf("clk_peri = %dkHz\r\n", f_clk_peri);
    fast_serial_printf("clk_usb = %dkHz\r\n", f_clk_usb);
    fast_serial_printf("clk_adc = %dkHz\r\n", f_clk_adc);
    
    #ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
        fast_serial_printf("clk_rtc = %dkHz\r\n", f_clk_rtc);
    #endif
}