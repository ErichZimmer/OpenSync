#pragma once

#include <stdbool.h>
#include <stdint.h>


#define MAX_CLOCK_SYNCS 1
#define MAX_PULSE_CHANNELS 8
#define MAX_PULSE_PAIR 6


enum {
    SCPI_GATE_DISABLED = 0,
    SCPI_GATE_PULSE,
    SCPI_GATE_OUTPUT,
    SCPI_GATE_CHANNEL,
    SCPI_GATE_MODE_END
};

enum { 
    SCPI_GATE_LOW = 0, 
    SCPI_GATE_HIGH,
    SCPI_GATE_LEVEL_END
};

enum { 
    SCPI_TRIGGER_DISABLED = 0, 
    SCPI_TRIGGER_ENABLED,
    SCPI_TRIGGER_MODE_END
};

enum {
    SCPI_TRIGGER_RISING = 0, 
    SCPI_TRIGGER_FALLING,
    SCPI_TRIGGER_EDGE_END
};

struct pulse_scpi_config
{
    uint32_t clock_divider;
    bool state;
    uint32_t level;
    double period;                 // Seconds, after SCPI suffix conversion
    uint32_t bcounter;            // T0: acquisition quota
    uint32_t pcounter;
    uint32_t ocounter;
    uint32_t wcounter;
    uint32_t tcounter;
    uint32_t gate_mode;
    uint32_t gate_level;
    uint32_t trig_mode;
    uint32_t trig_edge;
    uint32_t sync;
    bool state_buffer[MAX_PULSE_PAIR];
    double delay_buffer[MAX_PULSE_PAIR];
};
