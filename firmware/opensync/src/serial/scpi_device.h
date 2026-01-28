#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_DEVICE_COMMANDS \
    {.pattern = "DEVice:STATus?", .callback = SCPI_DeviceStatusQ,}, \
    {.pattern = "DEVice:DEBug", .callback = SCPI_DeviceDebug,}, \
    {.pattern = "DEVice:DEBug?", .callback = SCPI_DeviceDebugQ,}, \
    {.pattern = "DEVice:FREQuency?", .callback = SCPI_DeviceFrequencyQ,}, \
    {.pattern = "DEVice:START", .callback = SCPI_DeviceStart,}, \
    {.pattern = "DEVice:STOP", .callback = SCPI_DeviceStop,}, \
    {.pattern = "DEVice:RESet", .callback = SCPI_DeviceReset,}, \
    {.pattern = "DEVice:TEST?", .callback = SCPI_DeviceTestQ,}, \

scpi_result_t SCPI_DeviceStatusQ(
    scpi_t* context
);

scpi_result_t SCPI_DeviceDebug(
    scpi_t* context
);

scpi_result_t SCPI_DeviceDebugQ(
    scpi_t* context
);

scpi_result_t SCPI_DeviceFrequencyQ(
    scpi_t* context
);

scpi_result_t SCPI_DeviceStart(
    scpi_t* context
);

scpi_result_t SCPI_DeviceStop(
    scpi_t* context
);

scpi_result_t SCPI_DeviceReset(
    scpi_t* context
);

scpi_result_t SCPI_DeviceTestQ(
    scpi_t* context
);