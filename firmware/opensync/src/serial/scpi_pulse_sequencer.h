#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_PULSE_COMMANDS \
    {.pattern = "PULSe#:ENABle", .callback = SCPI_PulseEnable,}, \
    {.pattern = "PULSe#:DISAble", .callback = SCPI_PulseDisable,}, \
    {.pattern = "PULSe#:STATus?", .callback = SCPI_PulseStatusQ,}, \
    {.pattern = "PULSe#:DIVider", .callback = SCPI_PulseClockDivider,}, \
    {.pattern = "PULSe#:DIVider?", .callback = SCPI_PulseClockDividerQ,}, \
    {.pattern = "PULSe#:PIN", .callback = SCPI_PulsePin,}, \
    {.pattern = "PULSe#:PIN?", .callback = SCPI_PulsePinQ,}, \
    {.pattern = "PULSe#:INSTructions", .callback = SCPI_PulseInstructions,}, \
    {.pattern = "PULSe#:INSTructions?", .callback = SCPI_PulseInstructionsQ,}, \
    {.pattern = "PULSe#:RESet", .callback = SCPI_PulseReset,}, \

scpi_result_t SCPI_PulseEnable(
    scpi_t* context
);

scpi_result_t SCPI_PulseDisable(
    scpi_t* context
);

scpi_result_t SCPI_PulseStatusQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseClockDivider(
    scpi_t* context
);

scpi_result_t SCPI_PulseClockDividerQ(
    scpi_t* context
);

scpi_result_t SCPI_PulsePin(
    scpi_t* context
);

scpi_result_t SCPI_PulsePinQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseInstructions(
    scpi_t* context
);

scpi_result_t SCPI_PulseInstructionsQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseReset(
    scpi_t* context
);