#pragma once

#include "scpi/scpi.h"


#define INSTRUMENT_PULSE_COMMANDS \
    {.pattern = "PULSe#:DIVider",   .callback = SCPI_ClockDivider,}, \
    {.pattern = "PULSe#:DIVider?",  .callback = SCPI_ClockDividerQ,}, \
    {.pattern = "PULSe#:STATe",     .callback = SCPI_State,}, \
    {.pattern = "PULSe#:STATe?",    .callback = SCPI_StateQ,}, \
    {.pattern = "PULSe#:PERiod",    .callback = SCPI_Period,}, \
    {.pattern = "PULSe#:PERiod?",   .callback = SCPI_PeriodQ,}, \
    {.pattern = "PULSe#:BCOunter",  .callback = SCPI_BurstCounter,}, \
    {.pattern = "PULSe#:BCOunter?", .callback = SCPI_BurstCounterQ,}, \
    {.pattern = "PULSe#:PCOunter",  .callback = SCPI_PulseCounter,}, \
    {.pattern = "PULSe#:PCOunter?", .callback = SCPI_PulseCounterQ,}, \
    {.pattern = "PULSe#:OCOunter",  .callback = SCPI_OffCounter,}, \
    {.pattern = "PULSe#:OCOunter?", .callback = SCPI_OffCounterQ,}, \
    {.pattern = "PULSe#:SYNC",      .callback = SCPI_Sync,}, \
    {.pattern = "PULSe#:SYNC?",     .callback = SCPI_SyncQ,}, \
    {.pattern = "PULSe#:BUFfer",    .callback = SCPI_Buffer,}, \
    {.pattern = "PULSe#:BUFfer?",   .callback = SCPI_BufferQ,}, \
    {.pattern = "PULSe#:OUTPut:LEVel",  .callback = SCPI_ChannelOutputMode,}, \
    {.pattern = "PULSe#:OUTPut:LEVel?", .callback = SCPI_ChannelOutputModeQ,}, \
    {.pattern = "PULSe#:WCOunter",  .callback = SCPI_WaitCounter,}, \
    {.pattern = "PULSe#:WCOunter?", .callback = SCPI_WaitCounterQ,}, \
    {.pattern = "PULSe#:GATe:MODe", .callback = SCPI_GateMode,}, \
    {.pattern = "PULSe#:GATe:MODe?",.callback = SCPI_GateModeQ,}, \
    {.pattern = "PULSe#:GATe:LOGic",    .callback = SCPI_GateLogic,}, \
    {.pattern = "PULSe#:GATe:LOGic?",   .callback = SCPI_GateLogicQ,}, \
    {.pattern = "PULSe#:CGATe:MODe",    .callback = SCPI_ChannelGateMode,}, \
    {.pattern = "PULSe#:CGATe:MODe?",   .callback = SCPI_ChannelGateModeQ,}, \
    {.pattern = "PULSe#:CGATe:LOGic",   .callback = SCPI_ChannelGateLogic,}, \
    {.pattern = "PULSe#:CGATe:LOGic?",  .callback = SCPI_ChannelGateLogicQ,}, \
    {.pattern = "PULSe#:TRIGger:MODe",  .callback = SCPI_TriggerMode,}, \
    {.pattern = "PULSe#:TRIGger:MODe?", .callback = SCPI_TriggerModeQ,}, \
    {.pattern = "PULSe#:TRIGger:EDGe",  .callback = SCPI_TriggerEdge,}, \
    {.pattern = "PULSe#:TRIGger:EDGe?", .callback = SCPI_TriggerEdgeQ,}, \
    {.pattern = "PULSe#:RESet",         .callback = SCPI_Reset,}, \
    
struct pulse_scpi_config* sequencer_scpi_config_get();

void pulse_channels_clear();

scpi_result_t SCPI_State(
    scpi_t* context
);

scpi_result_t SCPI_StateQ(
    scpi_t* context
);

scpi_result_t SCPI_ClockDivider(
    scpi_t* context
);

scpi_result_t SCPI_ClockDividerQ(
    scpi_t* context
);

scpi_result_t SCPI_Period(
    scpi_t* context
);

scpi_result_t SCPI_PeriodQ(
    scpi_t* context
);

scpi_result_t SCPI_BurstCounter(
    scpi_t* context
);

scpi_result_t SCPI_BurstCounterQ(
    scpi_t* context
);

scpi_result_t SCPI_PulseCounter(
    scpi_t* context
);

scpi_result_t SCPI_PulseCounterQ(
    scpi_t* context
);

scpi_result_t SCPI_OffCounter(
    scpi_t* context
);

scpi_result_t SCPI_OffCounterQ(
    scpi_t* context
);

scpi_result_t SCPI_Sync(
    scpi_t* context
);

scpi_result_t SCPI_SyncQ(
    scpi_t* context
);

scpi_result_t SCPI_Buffer(
    scpi_t* context
);

scpi_result_t SCPI_BufferQ(
    scpi_t* context
);

scpi_result_t SCPI_ChannelOutputMode(
    scpi_t* context
);

scpi_result_t SCPI_ChannelOutputModeQ(
    scpi_t* context
);

scpi_result_t SCPI_WaitCounter(
    scpi_t* context
);

scpi_result_t SCPI_WaitCounterQ(
    scpi_t* context
);

scpi_result_t SCPI_GateMode(
    scpi_t* context
);

scpi_result_t SCPI_GateModeQ(
    scpi_t* context
);

scpi_result_t SCPI_GateLogic(
    scpi_t* context
);

scpi_result_t SCPI_GateLogicQ(
    scpi_t* context
);

scpi_result_t SCPI_ChannelGateMode(
    scpi_t* context
);

scpi_result_t SCPI_ChannelGateModeQ(
    scpi_t* context
);

scpi_result_t SCPI_ChannelGateLogic(
    scpi_t* context
);

scpi_result_t SCPI_ChannelGateLogicQ(
    scpi_t* context
);

scpi_result_t SCPI_ChannelGateLogic(
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

scpi_result_t SCPI_Reset(
    scpi_t* context
);