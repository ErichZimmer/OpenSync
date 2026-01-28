#include <stdio.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "scpi/scpi.h"

#include "version/opensync_version_info.h"


// Return firmware version
scpi_result_t SCPI_SystemFirmwareVersionQ(
    scpi_t* context
) {
    SCPI_ResultCharacters(
        context,
        OPENSYNC_FIRMWARE_VERSION,
        strlen(OPENSYNC_FIRMWARE_VERSION)
    );

    return SCPI_RES_OK;
}


// Return SCPI command version
scpi_result_t SCPI_SystemCommandsVersionQ(
    scpi_t* context
) {
    SCPI_ResultCharacters(
        context,
        OPENSYNC_COMMANDS_VERSION,
        strlen(OPENSYNC_COMMANDS_VERSION)
    );

    return SCPI_RES_OK;
}