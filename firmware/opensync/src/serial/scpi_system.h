#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_SYSTEM_COMMANDS \
    {.pattern = "SYSTem:FIRMware:VERSion?", .callback = SCPI_SystemFirmwareVersionQ,}, \
    {.pattern = "SYSTem:COMMands:VERSion?", .callback = SCPI_SystemCommandsVersionQ,}, \

scpi_result_t SCPI_SystemFirmwareVersionQ(
    scpi_t* context
);

scpi_result_t SCPI_SystemCommandsVersionQ(
    scpi_t* context
);