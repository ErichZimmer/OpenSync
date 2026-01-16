#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_SYST_COMMANDS \
    {.pattern = "SYSTem:STATus?", .callback = SCPI_SystemStatusQ,}, \
    {.pattern = "SYSTem:DEBug", .callback = SCPI_SystemDebug,}, \
    {.pattern = "SYSTem:DEBug?", .callback = SCPI_SystemDebugQ,}, \
    {.pattern = "SYSTem:FREQuency?", .callback = SCPI_SystemFrequencyQ,}, \
    {.pattern = "SYSTem:START", .callback = SCPI_SystemStart,}, \
    {.pattern = "SYSTem:STOP", .callback = SCPI_SystemStop,}, \
    {.pattern = "SYSTem:RESet", .callback = SCPI_SystemReset,}, \

scpi_result_t SCPI_SystemStatusQ(
    scpi_t* context
);

scpi_result_t SCPI_SystemDebug(
    scpi_t* context
);

scpi_result_t SCPI_SystemDebugQ(
    scpi_t* context
);

scpi_result_t SCPI_SystemFrequencyQ(
    scpi_t* context
);

scpi_result_t SCPI_SystemStart(
    scpi_t* context
);

scpi_result_t SCPI_SystemStop(
    scpi_t* context
);

scpi_result_t SCPI_SystemReset(
    scpi_t* context
);