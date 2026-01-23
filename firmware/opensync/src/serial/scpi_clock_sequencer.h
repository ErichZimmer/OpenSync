#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_CLOCK_COMMANDS \
    {.pattern = "CLOCk#:STATe", .callback = SCPI_ClockStatus,}, \
    {.pattern = "CLOCk#:State?", .callback = SCPI_ClockStatusQ,}, \
    {.pattern = "CLOCk#:DIVider", .callback = SCPI_ClockClockDivider,}, \
    {.pattern = "CLOCk#:DIVider?", .callback = SCPI_ClockClockDividerQ,}, \
    {.pattern = "CLOCk#:UNITs", .callback = SCPI_ClockeUnits,}, \
    {.pattern = "CLOCk#:UNIts?", .callback = SCPI_ClockUnitsQ,}, \
    {.pattern = "CLOCk:DATA:STOre:FREQ", .callback = SCPI_ClockDataFreq,}, \
    {.pattern = "CLOCk:DATA:STOre:FREQ?", .callback = SCPI_ClockDataFreqQ,}, \
    {.pattern = "CLOCk:DATA:STOre:REPs", .callback = SCPI_ClockDataReps,}, \
    {.pattern = "CLOCk:DATA:STOre:REPs?", .callback = SCPI_ClockDataRepsQ,}, \
    {.pattern = "CLOCk:DATA:STOre:CLEar", .callback = SCPI_ClockDataClear,}, \
    {.pattern = "CLOCk#:DATA:STOre:APPly", .callback = SCPI_ClockDataApply,}, \
    {.pattern = "CLOCk#:DATA?", .callback = SCPI_ClockDataQ,}, \
    {.pattern = "CLOCk#:TRIGger:MODe", .callback = SCPI_ClockTriggerMode,}, \
    {.pattern = "CLOCk#:TRIGger:MODe?", .callback = SCPI_ClockTriggerModeQ,}, \
    {.pattern = "CLOCk#:TRIGger:DATA", .callback = SCPI_ClockTriggerData,}, \
    {.pattern = "CLOCk#:TRIGger:DATA?", .callback = SCPI_ClockTriggerDataQ,}, \
    {.pattern = "CLOCk#:RESet", .callback = SCPI_ClockReset,}, \

void clock_sequencer_cache_clear();

scpi_result_t SCPI_ClockStatus(
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

scpi_result_t SCPI_ClockeUnits(
    scpi_t* context
);

scpi_result_t SCPI_ClockUnitsQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataReps(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataRepsQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataFreq(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataFreqQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataClear(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataApply(
    scpi_t* context
);

scpi_result_t SCPI_ClockDataQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerMode(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerModeQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerData(
    scpi_t* context
);

scpi_result_t SCPI_ClockTriggerDataQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockReset(
    scpi_t* context
);