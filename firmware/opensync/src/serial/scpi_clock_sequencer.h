#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_CLOCK_COMMANDS \
    {.pattern = "CLOCk#:ENABle", .callback = SCPI_ClockEnable,}, \
    {.pattern = "CLOCk#:DISAble", .callback = SCPI_ClockDisable,}, \
    {.pattern = "CLOCk#:STATus?", .callback = SCPI_ClockStatusQ,}, \
    {.pattern = "CLOCk#:DIVider", .callback = SCPI_ClockClockDivider,}, \
    {.pattern = "CLOCk#:DIVider?", .callback = SCPI_ClockClockDividerQ,}, \
    {.pattern = "CLOCk#:INSTructions", .callback = SCPI_ClockInstructions,}, \
    {.pattern = "CLOCk#:INSTructions?", .callback = SCPI_ClockInstructionsQ,}, \
    {.pattern = "CLOCk#:TRIGger:MODe", .callback = SCPI_ClockTriggerMode,}, \
    {.pattern = "CLOCk#:TRIGger:MODe?", .callback = SCPI_ClockTriggerModeQ,}, \
    {.pattern = "CLOCk#:TRIGger:INSTructions", .callback = SCPI_ClockTriggerInstructions,}, \
    {.pattern = "CLOCk#:TRIGger:INSTructions?", .callback = SCPI_ClockTriggerInstructionsQ,}, \
    {.pattern = "CLOCk#:RESet", .callback = SCPI_ClockReset,}, \

scpi_result_t SCPI_ClockEnable(
    scpi_t* context
);

scpi_result_t SCPI_ClockDisable(
    scpi_t* context
);

scpi_result_t SCPI_ClockStatusQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockClockDivider(
    scpi_t* context
);

scpi_result_t SCPI_ClockClockDividerQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockInstructions(
    scpi_t* context
);

scpi_result_t SCPI_ClockInstructionsQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerMode(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerModeQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerInstructions(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerInstructionsQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockReset(
    scpi_t* context
);