#include "core_1.h"

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "fast_serial.h"

#include "structs/clock_sequencer_config.h"
#include "structs/pulse_sequencer_config.h"
#include "structs/pulse_scpi_config.h"
#include "sequencer/sequencer_common.h"
#include "sequencer/sequencer_clock_internal.h"
#include "sequencer/sequencer_clock_t0.h"
#include "sequencer/sequencer_clock_triggers.h"
#include "sequencer/sequencer_outputs.h"
#include "status/sequencer_status.h"
#include "status/debug_status.h"
#include "serial/scpi_sequencer.h"

#include "sequencer_pio_clock_t0_gated.pio.h"
#include "sequencer_pio_output.pio.h"

enum { OUTPUT_CHANNELS = 8 };

static struct pulse_sequencer_config output_struct[OUTPUT_CHANNELS] = {0};

static struct clock_sequencer_config internal_clock = {
    .pio = pio2, 
    .sm = 0,
    .dma_chan = -1
};

static struct clock_sequencer_config t0_clock = {
    .pio = pio2, 
    .sm = 1,
    .dma_chan = -1
};

static int output_offset[2] = {-1, -1};
static int internal_offset = -1;
static int t0_offset = -1;

const uint32_t ARM_SEQUENCER = 1;
const uint32_t CORE_1_READY = 1;
const uint32_t CORE_1_FUCKED = 0;


void core_1_init(void)
{
    // Load the shared programs once, before waiting for ARM.
    output_offset[0] = sequencer_program_outputs_add(pio0);
    output_offset[1] = sequencer_program_outputs_add(pio1);
    internal_offset = sequencer_program_clock_internal_add(pio2);
    t0_offset = sequencer_program_clock_t0_add(pio2);

    bool initialized = output_offset[0] >= 0 && 
                       output_offset[1] >= 0 &&
                       internal_offset >= 0 &&
                       t0_offset >= 0;

    if (initialized)
    {
        internal_clock.program_offset = (uint)internal_offset;
        t0_clock.program_offset = (uint)t0_offset;

        for (uint i = 0; i < OUTPUT_CHANNELS; ++i)
        {
            initialized = sequencer_outputs_init(
                &output_struct[i],
                get_pio_mapping(i),
                i % 4u,
                (uint)output_offset[i / 4u],
                PINS_OUTPUT[i],
                PIN_CLOCK_T0
            );

            if (!initialized)
                break;
        }
    }

    // This should never happen, but just in case...
    if (!initialized)
    {
        sequencer_status_set(PROGRAM_FAILURE);

        multicore_fifo_push_blocking(CORE_1_FUCKED);

        while (true)
            tight_loop_contents();
    }

    multicore_fifo_push_blocking(CORE_1_READY);

    while (true)
    {
        uint32_t arming_status = multicore_fifo_pop_blocking();

        if (arming_status != ARM_SEQUENCER)
            continue;

        sequencer_status_set(ARMING);

        uint32_t debug_status_local = debug_status_get();
        
        if (debug_status_local == SEQUENCER_DEBUG)
        {
            fast_serial_printf("Internal Message: Arming aborted by debug level 1\r\n");
            sequencer_status_set(ABORTED);

            continue;
        }

        if (debug_status_local != SEQUENCER_DNDEBUG)
            fast_serial_printf("Internal Message: Configuring sequencers\r\n");

        const struct pulse_scpi_config* scpi_interface = sequencer_scpi_config_get();

        bool gpio_initialized = scpi_interface[0].state &&
            sequencer_clock_triggers_init(pio2, &scpi_interface[0]);

        // Configure, start, then wait for the last waveform to finish.
        bool success = gpio_initialized && sequencer_configure(scpi_interface);
        
        if (debug_status_local != SEQUENCER_DNDEBUG)
            fast_serial_printf("Internal Message: Starting sequencers\r\n");
        if (success)
            success = sequencer_start();

        if (debug_status_local != SEQUENCER_DNDEBUG)
            fast_serial_printf("Internal Message: Waiting on sequencers to finish\r\n");
        if (success)
            success = sequencer_wait();

        if (debug_status_local != SEQUENCER_DNDEBUG)
            fast_serial_printf("Internal Message: Freeing sequencers\r\n");

        if (!success || sequencer_status_get() == ABORT_REQUESTED)
            sequencer_status_set(ABORTING);
        else
            sequencer_status_set(DISARMING);

        sequencer_free();

        if (debug_status_local != SEQUENCER_DNDEBUG)
            fast_serial_printf("Internal Message: Freeing triggers\r\n");

        if (gpio_initialized)
            sequencer_clock_triggers_deinit(&scpi_interface[0]);

        if (sequencer_status_get() == ABORTING)
            sequencer_status_set(ABORTED);
        else
            sequencer_status_set(IDLE);

        if (debug_status_local != SEQUENCER_DNDEBUG)
            fast_serial_printf("Internal Message: Sequencer cleanup complete\r\n");
    }
}


PIO get_pio_mapping(uint channel)
{
    return channel < 4u ? pio0 : pio1;
}


bool sequencer_configure(
    const struct pulse_scpi_config* scpi_interface
) {
    if (sequencer_status_get() == ABORT_REQUESTED)
        return false;

    if (!sequencer_clock_internal_sm_config(&internal_clock, &scpi_interface[0]))
        return false;

    if (!sequencer_clock_t0_sm_config(&t0_clock, &scpi_interface[0]))
        return false;

    for (uint i = 0; i < OUTPUT_CHANNELS; ++i)
    {
        if (!scpi_interface[i + 1u].state)
            continue;

        if (sequencer_status_get() == ABORT_REQUESTED)
            return false;

        output_struct[i].gate_enabled =
            scpi_interface[0].gate_mode == SCPI_GATE_CHANNEL;

        if (!sequencer_outputs_sm_config(&output_struct[i], &scpi_interface[i + 1u]))
            return false;
    }
    return true;
}


bool sequencer_dma_failed(int channel)
{
    return channel >= 0 &&
        (dma_channel_hw_addr((uint)channel)->ctrl_trig &
         DMA_CH0_CTRL_TRIG_AHB_ERROR_BITS) != 0;
}


bool sequencer_run_ok()
{
    if (sequencer_status_get() == ABORT_REQUESTED ||
        sequencer_dma_failed(internal_clock.dma_chan))
        return false;

    for (uint i = 0; i < OUTPUT_CHANNELS; ++i) {
        if (output_struct[i].sm_claimed && sequencer_dma_failed(output_struct[i].dma_chan))
            return false;
    }

    return true;
}


bool sequencer_wait_for_event(
    PIO pio, 
    uint sm, 
    uint offset, 
    uint on_wait, 
    uint off_wait
) {
    // The producer must be stopped LOW before calling this function.
    // Reaching WAIT 1 means the preceding event/waveform has completed.
    while (sequencer_run_ok()) {
        const uint pc = pio_sm_get_pc(pio, sm);
        if (pc == offset + on_wait || pc == offset + off_wait)
            return true;

        sleep_us(100);
    }

    return false;
}


bool sequencer_wait_for_t0(void)
{
    return sequencer_wait_for_event(
        t0_clock.pio,
        t0_clock.sm, 
        t0_clock.program_offset,
        sequencer_pio_clock_t0_gated_offset_on_wait_high,
        sequencer_pio_clock_t0_gated_offset_off_wait_high);
}


bool sequencer_wait_for_output(uint channel)
{
    const struct pulse_sequencer_config* output = &output_struct[channel];
    
    // The producer must be stopped LOW before waiting for the output.
    while (sequencer_run_ok()) {
        const bool dma_finished =
            !dma_channel_is_busy((uint)output->dma_chan) &&
            dma_channel_hw_addr((uint)output->dma_chan)->transfer_count == 0u &&
            pio_sm_is_tx_fifo_empty(output->pio, output->sm);

        const uint pc = pio_sm_get_pc(output->pio, output->sm);

        if (pc == output->program_offset + sequencer_pio_output_offset_on_wait_high ||
            pc == output->program_offset + sequencer_pio_output_offset_off_wait_high)
            return true;

        // An exhausted bcounter stalls before the next waveform is pulled.
        if (dma_finished &&
            pc == output->program_offset + sequencer_pio_output_offset_instruction_pull)
            return true;

        sleep_us(100);
    }

    return false;
}


bool sequencer_start()
{
    uint32_t output_masks[2] = {0};

    // T0 and the internal clock are still LOW. Prime all GPIO consumers.
    for (uint i = 0; i < OUTPUT_CHANNELS; ++i) {
        if (!output_struct[i].configured)
            continue;

        output_masks[i / 4u] |= 1u << output_struct[i].sm;

        pio_sm_set_enabled(
            output_struct[i].pio, 
            output_struct[i].sm, 
            true
        );

        if (!sequencer_wait_for_output(i))
            return false;

        pio_sm_set_enabled(
            output_struct[i].pio, 
            output_struct[i].sm, 
            false
        );
    }

    pio_sm_set_enabled(
        t0_clock.pio, 
        t0_clock.sm, 
        true
    );

    if (!sequencer_wait_for_t0())
        return false;

    pio_sm_set_enabled(
        t0_clock.pio, 
        t0_clock.sm, 
        false
    );

    if (!sequencer_run_ok()) 
        return false;

    sequencer_status_set(RUNNING);

    // Restart clock dividers and enable all three PIO blocks together.
    pio_enable_sm_multi_mask_in_sync(
        pio1,
         output_masks[0], 
         output_masks[1],
        (1u << internal_clock.sm) | (1u << t0_clock.sm)
    );
    
    return true;
}


bool sequencer_wait(void)
{
    uint32_t active = 0u;
    uint32_t done = 0u;
    uint max_divider = t0_clock.clock_divider;

    for (uint i = 0; i < OUTPUT_CHANNELS; ++i) {
        if (!output_struct[i].configured)
            continue;

        if (!output_struct[i].sm_claimed || output_struct[i].dma_chan < 0)
            return false;

        active |= 1u << i;

        if (output_struct[i].clock_divider > max_divider)
            max_divider = output_struct[i].clock_divider;
    }

    // Two SM periods plus input-settling margin.
    const uint64_t settle_us =
        (uint64_t)(2.0 * CLOCK_CYCLE_NS * max_divider / 1000.0) + 2u;

    while (sequencer_run_ok() &&
           !sequencer_clock_internal_finished(&internal_clock)) {
        sleep_us(100);
    }

    if (!sequencer_run_ok())
        return false;

    sleep_us(settle_us);

    if (!sequencer_wait_for_t0())
        return false;

    while (done != active) {
        const uint32_t previous_done = done;

        for (uint i = 0; i < OUTPUT_CHANNELS; ++i) {
            const uint32_t bit = 1u << i;

            if (!(active & bit) || (done & bit))
                continue;

            const struct pulse_sequencer_config* output = &output_struct[i];

            bool source_done = output->pin_trigger == t0_clock.pin_out;

            for (uint j = 0; j < OUTPUT_CHANNELS && !source_done; ++j) {
                source_done =
                    (done & (1u << j)) != 0u &&
                    output->pin_trigger == output_struct[j].pin_out;
            }

            if (!source_done)
                continue;

            sleep_us(settle_us);

            // Existing helper checks WAIT_HIGH or exhausted B-counter DMA.
            if (!sequencer_wait_for_output(i))
                return false;

            done |= bit;
        }

        // No eligible channel means a cycle or unavailable SYNC source.
        if (done == previous_done)
            return false;
    }

    return sequencer_run_ok();
}


void sequencer_free()
{
    // Stop every owned SM before changing GPIO levels or DMA state.
    if (internal_clock.sm_claimed)
        pio_sm_set_enabled(
            internal_clock.pio, 
            internal_clock.sm, 
            false
        );

    if (t0_clock.sm_claimed)
        pio_sm_set_enabled(
            t0_clock.pio, 
            t0_clock.sm, 
            false
        );
    
    for (uint i = 0; i < OUTPUT_CHANNELS; ++i)
    {
        if (output_struct[i].sm_claimed)
            pio_sm_set_enabled(
                output_struct[i].pio,
                output_struct[i].sm,
                false
            );
    }

    for (uint i = 0; i < OUTPUT_CHANNELS; ++i)
        sequencer_outputs_free(&output_struct[i]);

    sequencer_clock_t0_free(&t0_clock);
    sequencer_clock_internal_free(&internal_clock);
}
