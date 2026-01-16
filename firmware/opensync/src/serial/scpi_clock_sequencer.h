#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_PULS_COMMANDS \
    {.pattern = "CLOCk#:ENABle", .callback = SCPI_ClockEnable,}, \
    {.pattern = "CLOCk#:DISAble", .callback = SCPI_ClockDisable,}, \
    {.pattern = "CLOCk#:STATus?", .callback = SCPI_ClockStatusQ,}, \
    {.pattern = "CLOCk#:DIVider", .callback = SCPI_ClockClockDivider,}, \
    {.pattern = "CLOCk#:DIVider?", .callback = SCPI_ClockClockDividerQ,}, \
    {.pattern = "CLOCk#:INSTructions", .callback = SCPI_ClockInstructions,}, \
    {.pattern = "CLOCk#:INSTructions?", .callback = SCPI_ClockInstructionsQ,}, \
    {.pattern = "CLOCk#:TRIGger:MODe?", .callback = SCPI_ClockTriggerModeQ,}, \
    {.pattern = "CLOCk#:TRIGger:MODe:INTernal", .callback = SCPI_ClockTriggerInternal,}, \
    {.pattern = "CLOCk#:TRIGger:MODe:RISing", .callback = SCPI_ClockTriggerRisingEdge,}, \
    {.pattern = "CLOCk#:TRIGger:INSTructions", .callback = SCPI_ClockTriggerInstructions,}, \
    {.pattern = "CLOCk#:TRIGger:INSTructions?", .callback = SCPI_ClockTriggerInstructionsQ,}, \
    {.pattern = "CLOCk#:RESet", .callback = SCPI_PulseReset,}, \

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

scpi_result_t SCPI_ClockTriggerModeQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerInternal(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerRisingEdge(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerInstructions(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerInstructionsQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseReset(
    scpi_t* context
);