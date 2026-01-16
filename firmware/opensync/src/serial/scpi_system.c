#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

#include "scpi/scpi.h"

#include "system/core_1.h"
#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "scpi_common.h"


// Return system status
scpi_result_t SCPI_SystemStatusQ(
    scpi_t* context
) {
    uint32_t status_copy = sequencer_status_get();

    const char* status_text = sequencer_status_to_str(status_copy);

    SCPI_ResultCharacters(
        context,
        status_text,
        strlen(status_text)
    );

    return SCPI_RES_OK;
}


// Set debug status
scpi_result_t SCPI_SystemDebug(
    scpi_t* context
) {
    // Define variables
    uint32_t debug_status_local = 0;

    // Get system status
    uint32_t status_copy = sequencer_status_get();

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if( !((status_copy == IDLE) || (status_copy == ABORTED )))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_SYSTEM_ERROR
        );

        return SCPI_RES_ERR;
    }

    // Retrieve the debug status number if present
    if( !SCPI_ParamUInt32(context, &debug_status_local, TRUE))
    {
        return SCPI_RES_ERR;
    }

    // Check that the debug status is within reason
    // TODO: include debug status validation function
    if (debug_status_local > SEQUENCER_DEBUG_ALL)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    // Now set debug status
    debug_status_set(debug_status_local);

    return SCPI_RES_OK;
}


// Return debug status
scpi_result_t SCPI_SystemDebugQ(
    scpi_t* context
) {
    uint32_t debug_status_copy = debug_status_get();

    const char* status_text = debug_status_to_str(debug_status_copy);

    SCPI_ResultCharacters(
        context,
        status_text,
        strlen(status_text)
    );

    return SCPI_RES_OK;
}


// Return system frequencies
scpi_result_t SCPI_SystemFrequencyQ(
    scpi_t* context
) {
    // local buffer for sprintf due to scpi limitations
    char output_buffer[64];

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

    // We can't use %d, so printf to local buffer then send to SCPI interface
    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "pll_sys = %ukHz\r\n", f_pll_sys
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "pll_usb = %ukHz\r\n", f_pll_usb
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "rosc = %ukHz\r\n", f_rosc
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "clk_sys = %ukHz\r\n", f_clk_sys
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "clk_peri = %ukHz\r\n", f_clk_peri
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "clk_usb = %ukHz\r\n", f_clk_usb
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "clk_adc = %ukHz\r\n", f_clk_adc
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );

    #ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
    snprintf(
        output_buffer, 
        sizeof output_buffer,
        "clk_rtc = %ukHz\r\n", f_clk_rtc
    );
    SCPI_ResultCharacters(
        context, 
        output_buffer,
        strlen(output_buffer)
    );
    #endif

    return SCPI_RES_OK;
}


// Start pulse sequence with current device settings
scpi_result_t SCPI_SystemStart(
    scpi_t* context
) {
    // Get system status
    uint32_t status_copy = sequencer_status_get();

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if( !((status_copy == IDLE) || (status_copy == ABORTED)))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_SYSTEM_ERROR
        );

        return SCPI_RES_ERR;
    }

    // Push arming status to sequencer core
    multicore_fifo_push_blocking(ARM_SEQUENCER);

    // TODO: wait and check for status change?

    return SCPI_RES_OK;
}


// Stop pulse sequence with current device settings
scpi_result_t SCPI_SystemStop(
    scpi_t* context
) {
    // Get system status
    uint32_t status_copy = sequencer_status_get();

    // If the system status *is* 0 (IDLE) or 5 (ABORTED), return an error
    if( ((status_copy == IDLE) || (status_copy == ABORTED)))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_SYSTEM_ERROR
        );

        return SCPI_RES_ERR;
    }

    // Set status to ABORT
    sequencer_status_set(ABORT_REQUESTED);

    // Wait a few milliseconds for system to disarm
    sleep_ms(10); // pause for 10 milliseconds

    // Get new system status
    status_copy = sequencer_status_get();

    // Now check if the system is IDLE or ABORTED
    if( !((status_copy == IDLE) || (status_copy == ABORTED)))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_SYSTEM_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Reset device
scpi_result_t SCPI_SystemReset(
    scpi_t* context
) {
    // Get system status
    uint32_t status_copy = sequencer_status_get();

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if ( !((status_copy == IDLE) || (status_copy == ABORTED)))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_SYSTEM_ERROR
        );

        return SCPI_RES_ERR;
    }

    // Reset clock containers
    for(uint32_t clock_id = 0; clock_id < CLOCKS_MAX; clock_id++)
    {
        clock_sequencer_state_reset(
            clock_id
        );
    }

    // Reset pulse containers
    for(uint32_t pulse_id = 0; pulse_id < CLOCKS_MAX; pulse_id++)
    {
        pulse_sequencer_state_reset(
            pulse_id
        );
    }

    // Finally, set sebug and sequencer status to default
    // TODO: Add reset functions for each
    sequencer_status_set(IDLE);
    debug_status_set(SEQUENCER_DNDEBUG);

    return SCPI_RES_OK;
}