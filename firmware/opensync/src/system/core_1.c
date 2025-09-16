#include "core_1.h"

uint32_t clock_instructions[CLOCKS_MAX][CLOCK_INSTRUCTIONS_MAX];
uint32_t trigger_instructions[CLOCKS_MAX][CLOCK_TRIGGERS_MAX];
uint32_t pulse_instructions[CLOCKS_MAX][PULSE_INSTRUCTIONS_MAX];

uint32_t trigger_pins[CLOCKS_MAX] = {13};

struct clock_config sequencer_clock_config[CLOCKS_MAX];
struct pulse_config sequencer_pulse_config[CLOCKS_MAX];

PIO pio_clocks = pio0;
PIO pio_output = pio1;

uint clock_divider = 1;
uint reps = 5;

const uint32_t ARM_SEQUENCER = 1;


void core_1_init()
{
    // Add a PIO programs
    uint offset_clock_freerun = sequencer_program_freerun_add(
        pio_clocks
    );

    uint offset_clock_triggered = sequencer_program_triggered_add(
        pio_clocks
    );

    // Add a single instance of the PIO pulse output program
    uint offset_output = sequencer_program_output_add(
        pio_output
    );

    // Make an array of ones to insert a dummy program 
    uint32_t dummy_clock_instruction[CLOCK_INSTRUCTIONS_MAX] = {0};

    // Add some random instructions to output
    /*
    Important Note:
    The clock output instruction are broken up into two parts: the pulse
    frequency and the amount of repitions to perform. For instance, every even 
    instruction starting from zero (e.g., 0, 2, 4, 6, 8...) sets the pulse
    repition amount. The repitition frequency is set at every odd instruction
    index (e.g., 1, 3, 5, 7, 9...).

    A repition instruction of zero skips the instruction pair, so zeros are okay here.
    */


    // Create some clock pattern
    dummy_clock_instruction[0] = reps; // The amount of reps to perform
    dummy_clock_instruction[1] = 400000000; // the repition rate (must be higher than the total duration of the pulse sequencer)

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

    // Set all delay instructions to 1 (they can't be zero)
    for (int i = 1; i < PULSE_INSTRUCTIONS_MAX; i += 2)
    {
        dummy_output_instruction[i] = 1; // 1 cycle delay
    }

    // Create some pulse pattern
    dummy_output_instruction[0] = 2048; // Channel 12 square wave
    dummy_output_instruction[1] = 40000000;

    dummy_output_instruction[0] = 1024; // Channel 11 square wave
    dummy_output_instruction[1] = 40000000;

    dummy_output_instruction[0] = 512; // Channel 10 square wave
    dummy_output_instruction[1] = 40000000;

    dummy_output_instruction[0] = 2560; // Channel 10 and 12
    dummy_output_instruction[1] = 40000000;


    // Make sure the final instructions are zero
    dummy_output_instruction[PULSE_INSTRUCTIONS_MAX - 1] = 0;
    dummy_output_instruction[PULSE_INSTRUCTIONS_MAX - 2] = 0;

    multicore_fifo_push_blocking(0);

    while(true)
    {
        uint32_t arming_status = multicore_fifo_pop_blocking();

        if (arming_status != ARM_SEQUENCER)
        {
            continue;
        }

        // INTERNAL CLOCK //
        sequencer_clocks_init(
            sequencer_clock_config,
            pio_clocks
        );

        // Add instruction to ouput channel 0
        sequencer_clock_insert_instructions_internal(
            &sequencer_clock_config[0],
            dummy_clock_instruction
        );

        sequencer_clock_configure(
            &sequencer_clock_config[0],
            INTERNAL_CLOCK_PINS[0],
            EXTERNAL_TRIGGER_PINS[0]
        );

        sequencer_clock_freerun_sm_config(
            &sequencer_clock_config[0],
            offset_clock_freerun,
            clock_divider
        );

        // OUTPUT //
        sequencer_output_init(
            sequencer_pulse_config,
            pio_output
        );

        // Add instruction to ouput channel 0
        sequencer_output_insert_instructions(
            &sequencer_pulse_config[0],
            dummy_output_instruction
        );

        // Set clock ID to 0
        sequencer_output_configure(
            &sequencer_pulse_config[0],
            INTERNAL_CLOCK_PINS[0]
        );

        sequencer_output_sm_config(
            &sequencer_pulse_config[0],
            offset_output,
            clock_divider,
            reps
        );

        uint pio_clocks_sm_mask = 0;
        uint pio_output_sm_mask = 0;

        for (uint32_t i = 0; i < CLOCKS_MAX; i++)
        {
            if (sequencer_pulse_config[i].active == true)
            {
                pio_clocks_sm_mask |= 1u << i;
            }
        }

        for (uint32_t i = 0; i < CLOCKS_MAX; i++)
        {
            if (sequencer_pulse_config[i].active == true)
            {
                pio_output_sm_mask |= 1u << i;
            }
        }

        pio_enable_sm_multi_mask_in_sync(
            pio_clocks,
            0u,
            pio_clocks_sm_mask,
            pio_output_sm_mask
        );

        // Stall core until all processes are done
        for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
        {
            if (sequencer_pulse_config[i].active != true)
            {
                continue;
            }

            while(dma_channel_is_busy(sequencer_pulse_config[i].dma_chan)){ }
        }

        // Cleanup clock configs
        for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
        {
            if (sequencer_clock_config[i].active == true)
            {
                sequencer_clock_sm_free(
                    &sequencer_clock_config[i]
                );
            }
        }

        // Cleanup output configs
        for (uint32_t i = 0; i < CLOCKS_MAX; ++i)
        {
            if (sequencer_pulse_config[i].active == true)
            {
                sequencer_output_sm_free(
                    &sequencer_pulse_config[i]
                );
            }
        }
    }
}