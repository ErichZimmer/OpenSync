#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_PULSE_COMMANDS \
    {.pattern = "SOURce:PULSe:SELect",    .callback = SCPI_PulseIndex,}, \
    {.pattern = "SOURce:PULSe:SELect?",   .callback = SCPI_PulseIndexQ,}, \
    {.pattern = "SOURce:PULSe#:STATe",    .callback = SCPI_PulseStatus,}, \
    {.pattern = "SOURce:PULSe#:STATe?",   .callback = SCPI_PulseStatusQ,}, \
    {.pattern = "SOURce:PULSe#:INPut",    .callback = SCPI_PulsePin,}, \
    {.pattern = "SOURce:PULSe#:INPut?",   .callback = SCPI_PulsePinQ,}, \
    {.pattern = "SOURce:PULSe#:DIVider",  .callback = SCPI_PulseClockDivider,}, \
    {.pattern = "SOURce:PULSe#:DIVider?", .callback = SCPI_PulseClockDividerQ,}, \
    {.pattern = "SOURce:PULSe#:UNITs",    .callback = SCPI_PulseUnits,}, \
    {.pattern = "SOURce:PULSe#:UNITs?",   .callback = SCPI_PulseUnitsQ,}, \
    {.pattern = "SOURce:PULSe#:DATA?",    .callback = SCPI_PulseDataQ,}, \
    {.pattern = "SOURce:PULSe#:RESet",    .callback = SCPI_PulseReset,}, \
    {.pattern = "SOURce:PULSe#:DATA:BUFFer:OUTPut",  .callback = SCPI_PulseDataOutput,}, \
    {.pattern = "SOURce:PULSe#:DATA:BUFFer:OUTPut?", .callback = SCPI_PulseDataOutputQ,}, \
    {.pattern = "SOURce:PULSe#:DATA:BUFFer:DELay",   .callback = SCPI_PulseDataDelay,}, \
    {.pattern = "SOURce:PULSe#:DATA:BUFFer:DELay?",  .callback = SCPI_PulseDataDelayQ,}, \
    {.pattern = "SOURce:PULSe#:DATA:BUFFer:CLEar",   .callback = SCPI_PulseDataClear,}, \
    {.pattern = "SOURce:PULSe#:DATA:BUFFer:APPly",   .callback = SCPI_PulseDataApply,}, \

void pulse_sequencer_cache_clear();

scpi_result_t SCPI_PulseIndex(
    scpi_t* context
);

scpi_result_t SCPI_PulseIndexQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseStatus(
    scpi_t* context
);

scpi_result_t SCPI_PulseStatusQ(
    scpi_t* context
);

scpi_result_t SCPI_PulsePin(
    scpi_t* context
);

scpi_result_t SCPI_PulsePinQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseClockDivider(
    scpi_t* context
);

scpi_result_t SCPI_PulseClockDividerQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseUnits(
    scpi_t* context
);

scpi_result_t SCPI_PulseUnitsQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataOutput(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataOutputQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataDelay(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataDelayQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataClear(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataApply(
    scpi_t* context
);

scpi_result_t SCPI_PulseDataQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseReset(
    scpi_t* context
);