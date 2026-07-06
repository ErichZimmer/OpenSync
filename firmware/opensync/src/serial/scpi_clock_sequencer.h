#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_CLOCK_COMMANDS \
    {.pattern = "SOURce:CLOCk:SELect",      .callback = SCPI_ClockIndex,}, \
    {.pattern = "SOURce:CLOCk:SELect?",     .callback = SCPI_ClockIndexQ,}, \
    {.pattern = "SOURce:CLOCk#:STATe",      .callback = SCPI_ClockStatus,}, \
    {.pattern = "SOURce:CLOCk#:State?",     .callback = SCPI_ClockStatusQ,}, \
    {.pattern = "SOURce:CLOCk#:DIVider",    .callback = SCPI_ClockClockDivider,}, \
    {.pattern = "SOURce:CLOCk#:DIVider?",   .callback = SCPI_ClockClockDividerQ,}, \
    {.pattern = "SOURce:CLOCk#:MODe",       .callback = SCPI_ClockMode,}, \
    {.pattern = "SOURce:CLOCk#:MODe?",      .callback = SCPI_ClockModeQ,}, \
    {.pattern = "SOURce:CLOCk#:UNITs",      .callback = SCPI_ClockeUnits,}, \
    {.pattern = "SOURce:CLOCk#:UNIts?",     .callback = SCPI_ClockUnitsQ,}, \
    {.pattern = "SOURce:CLOCk#:DATA?",      .callback = SCPI_ClockDataQ,}, \
    {.pattern = "SOURce:CLOCk#:RESet",      .callback = SCPI_ClockReset,}, \
    {.pattern = "SOURce:CLOCk#:DATA:BUFFer:FREQuency",  .callback = SCPI_ClockDataFreq,}, \
    {.pattern = "SOURce:CLOCk#:DATA:BUFFer:FREQuency?", .callback = SCPI_ClockDataFreqQ,}, \
    {.pattern = "SOURce:CLOCk#:DATA:BUFFer:COUNt",      .callback = SCPI_ClockDataReps,}, \
    {.pattern = "SOURce:CLOCk#:DATA:BUFFer:COUNt?",     .callback = SCPI_ClockDataRepsQ,}, \
    {.pattern = "SOURce:CLOCk#:DATA:BUFFer:CLEar",      .callback = SCPI_ClockDataClear,}, \
    {.pattern = "SOURce:CLOCk#:DATA:BUFFer:APPly",      .callback = SCPI_ClockDataApply,}, \
    {.pattern = "TRIGger:CLOCk#:MODe",          .callback = SCPI_TriggerMode,}, \
    {.pattern = "TRIGger:CLOCk#:MODe?",         .callback = SCPI_TriggerModeQ,}, \
    {.pattern = "TRIGger:CLOCk#:EDGE",          .callback = SCPI_TriggerEdge,}, \
    {.pattern = "TRIGger:CLOCk#:EDGE?",         .callback = SCPI_TriggerEdgeQ,}, \
    {.pattern = "TRIGger:CLOCk#:GATE:LEVel",    .callback = SCPI_TriggerLevel,}, \
    {.pattern = "TRIGger:CLOCk#:GATE:LEVel?",   .callback = SCPI_TriggerLevelQ,}, \
    {.pattern = "TRIGger:CLOCk#:UNITs",         .callback = SCPI_TriggerUnits,}, \
    {.pattern = "TRIGger:CLOCk#:UNITs?",        .callback = SCPI_TriggerUnitsQ,}, \
    {.pattern = "TRIGger:CLOCk#:INPut",         .callback = SCPI_TriggerPin,}, \
    {.pattern = "TRIGger:CLOCk#:INPut?",        .callback = SCPI_TriggerPinQ,}, \
    {.pattern = "TRIGger:CLOCk#:DELay",         .callback = SCPI_TriggerDelay,}, \
    {.pattern = "TRIGger:CLOCk#:DELay?",        .callback = SCPI_TriggerDelayQ,}, \
    {.pattern = "TRIGger:CLOCk#:SKIP",          .callback = SCPI_TriggerSkip,}, \
    {.pattern = "TRIGger:CLOCk#:SKIP?",         .callback = SCPI_TriggerSkipQ,}, \
    {.pattern = "TRIGger:CLOCk#:COUNt",         .callback = SCPI_TriggerCount,}, \
    {.pattern = "TRIGger:CLOCk#:COUNt?",        .callback = SCPI_TriggerCountQ,}, \
    
void clock_sequencer_cache_clear();

scpi_result_t SCPI_ClockIndex(
    scpi_t* context
);

scpi_result_t SCPI_ClockIndexQ(
    scpi_t* context
);

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

scpi_result_t SCPI_ClockMode(
    scpi_t* context
);

scpi_result_t SCPI_ClockModeQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerMode(
    scpi_t* context
);

scpi_result_t SCPI_TriggerModeQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerEdge(
    scpi_t* context
);

scpi_result_t SCPI_TriggerEdgeQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerLevel(
    scpi_t* context
);

scpi_result_t SCPI_TriggerLevelQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerPin(
    scpi_t* context
);

scpi_result_t SCPI_TriggerPinQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerUnits(
    scpi_t* context
);

scpi_result_t SCPI_TriggerUnitsQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerSkip(
    scpi_t* context
);

scpi_result_t SCPI_TriggerSkipQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerDelay(
    scpi_t* context
);

scpi_result_t SCPI_TriggerDelayQ(
    scpi_t* context
);

scpi_result_t SCPI_TriggerCount(
    scpi_t* context
);

scpi_result_t SCPI_TriggerCountQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockReset(
    scpi_t* context
);