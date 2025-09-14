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
    // Add a single instance of the freerun PIO program
    uint offset_clock = sequencer_program_freerun_add(
        pio_clocks
    );

    // Add a single instance of the PIO pulse output program
    uint offset_output = sequencer_program_output_add(
        pio_output
    );

    // Make an array of ones to insert a dummy program
    uint32_t dummy_output_instruction[PULSE_INSTRUCTIONS_MAX] = {0};

    // Add some random instructions to output
    /*
    Important Note:
    The pulse output instruction are broken up into two parts: the output
    state and the delay duration to the next instruction. For instance,
    every even instruction starting from zero (e.g., 0, 2, 4, 6, 8...) sets
    the output state. Only the first 12 bits are used to set the output state
    while the rest are discarded. The delay instructions are at every odd
    instruction index (e.g., 1, 3, 5, 7, 9...) and are used in a delay loop.
    The final two instruction at the end of the instruction buffer are used
    to set the output state to idle and a terminating flag.
    
    // DO NOT FORGET TO SET THE TERMINATING FLAGS (0) AND MAKE SURE ALL OTHER INSTRUCTIONS ARE NON-ZERO!!!!
    */

}