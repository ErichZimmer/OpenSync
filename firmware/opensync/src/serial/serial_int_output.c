#include "serial_interface.h"
#include "status/sequencer_status.h"


// Dump firmware version
void serial_print_version()
{
    fast_serial_printf("Version: %s\r\n", OPENSYNC_VERSION);
}

// Dump system status
void serial_print_status()
{
    uint32_t status_copy = sequencer_status_get();

    fast_serial_printf("Run status: %s\r\n", sequencer_status_to_str(status_copy));
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