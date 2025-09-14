#include "core_1.h"

uint32_t clock_instructions[CLOCKS_MAX][CLOCK_INSTRUCTIONS_MAX];
uint32_t trigger_instructions[CLOCKS_MAX][CLOCK_TRIGGERS_MAX];
uint32_t pulse_instructions[CLOCKS_MAX][PULSE_INSTRUCTIONS_MAX];

uint32_t trigger_pins[CLOCKS_MAX] = {13};

struct clock_config sequencer_clock_config[CLOCKS_MAX];
struct pulse_config sequencer_pulse_config[CLOCKS_MAX];

uint clocks_program_offsets[CLOCKS_MAX] = {0};
uint output_program_offsets[CLOCKS_MAX] = {0};

PIO pio_clocks = pio0;
PIO pio_output = pio1;


void core_1_init()
{
//    uint offset_clock = pio_add_program(
//        pio_clocks,
//
//    );
}