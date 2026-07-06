#pragma once 

#include <stdint.h>
#include "hardware/pio.h"

#define CLOCK_INSTRUCTIONS_MAX 16
// TODO: Rename this
#define CLOCK_TRIGGERS_MAX 2
#define CLOCKS_MAX 3
#define TRIGGERS_MAX 1

typedef enum {
    CLOCK_FREERUN = 0,
    CLOCK_TRIGGERED,
    CLOCK_TRIGGERED_RISING,
    CLOCK_TRIGGERED_HIGH,
    CLOCK_TRIGGERED_FALLING,
    CLOCK_TRIGGERED_LOW,
    CLOCK_TRIGGERED_SNIFFER
} clock_pio_program_t;

typedef enum {
    CLOCK_SCPI_MODE_FREERUN = 0,
    CLOCK_SCPI_MODE_SEQUENCER
} clock_scpi_mode_t;

typedef enum {
    CLOCK_TRIG_SOURCE_IMMEDIATE = 0,
    CLOCK_TRIG_SOURCE_EDGE,
    CLOCK_TRIG_SOURCE_GATE,
} clock_scpi_trigger_source_t;

typedef enum {
    CLOCK_TRIG_EDGE_POSITIVE = 0,
    CLOCK_TRIG_EDGE_NEGATIVE
} clock_scpi_trigger_edge_t;

typedef enum {
    CLOCK_GATE_LEVEL_HIGH = 0,
    CLOCK_GATE_LEVEL_LOW
} clock_scpi_gate_level_t;

typedef struct {
    clock_scpi_mode_t clock_mode;
    clock_scpi_trigger_source_t trigger_source;
    clock_scpi_trigger_edge_t edge;
    clock_scpi_gate_level_t gate_level;
} clock_scpi_config_t;

struct clock_config
{
    PIO pio;
    uint sm;
    uint program_offset;
    int dma_chan;
    uint32_t clock_pin;
    uint32_t trigger_pin;
    uint32_t clock_mode;
    uint32_t trigger_source;
    uint32_t trigger_edge;
    uint32_t trigger_level;
    uint32_t __attribute__((aligned(CLOCK_INSTRUCTIONS_MAX * sizeof(uint32_t)))) instructions[CLOCK_INSTRUCTIONS_MAX];
    uint32_t __attribute__((aligned(CLOCK_TRIGGERS_MAX     * sizeof(uint32_t)))) trigger_config[CLOCK_TRIGGERS_MAX];
    uint32_t trigger_reps;
    uint32_t clock_divider;
    double unit_offset;
    double unit_offset_trigger;
    bool active;
    bool configured;
};