#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_PULSE_COMMANDS \
    {.pattern = "PULSe:SELect", .callback = SCPI_PulseIndex,}, \
    {.pattern = "PULSe:SELect?", .callback = SCPI_PulseIndexQ,}, \
    {.pattern = "PULSe#:STATe", .callback = SCPI_PulseStatus,}, \
    {.pattern = "PULSe#:STATe?", .callback = SCPI_PulseStatusQ,}, \
    {.pattern = "PULSe#:PIN", .callback = SCPI_PulsePin,}, \
    {.pattern = "PULSe#:PIN?", .callback = SCPI_PulsePinQ,}, \
    {.pattern = "PULSe#:DIVider", .callback = SCPI_PulseClockDivider,}, \
    {.pattern = "PULSe#:DIVider?", .callback = SCPI_PulseClockDividerQ,}, \
    {.pattern = "PULSe#:UNITs", .callback = SCPI_PulseUnits,}, \
    {.pattern = "PULSe#:UNITs?", .callback = SCPI_PulseUnitsQ,}, \
    {.pattern = "PULSe#:DATA?", .callback = SCPI_PulseDataQ,}, \
    {.pattern = "PULSe#:RESet", .callback = SCPI_PulseReset,}, \
    {.pattern = "PULSe:DATA:STOre:OUTPuts", .callback = SCPI_PulseDataOutput,}, \
    {.pattern = "PULSe:DATA:STOre:OUTPuts?", .callback = SCPI_PulseDataOutputQ,}, \
    {.pattern = "PULSe:DATA:STOre:DELays", .callback = SCPI_PulseDataDelay,}, \
    {.pattern = "PULSe:DATA:STOre:DELays?", .callback = SCPI_PulseDataDelayQ,}, \
    {.pattern = "PULSe:DATA:STOre:CLEar", .callback = SCPI_PulseDataClear,}, \
    {.pattern = "PULSe:DATA:STOre:APPly", .callback = SCPI_PulseDataApply,}, \

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