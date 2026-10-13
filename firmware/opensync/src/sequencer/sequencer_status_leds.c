#include "sequencer_status_leds.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/types.h"

#include "sequencer/sequencer_common.h"
#include "status/sequencer_status.h"
#include "structs/pulse_scpi_config.h"


static repeating_timer_t program_led_timer;
static volatile uint32_t program_led_status = UINT32_MAX;
static volatile uint32_t program_led_tick = 0;


// We have to rely on timers and interrupts since PWM does not have
// the capacity for seconds long pulses.
static bool sequencer_leds_program_timer(
    repeating_timer_t* timer
) {
    (void) timer;

    ++program_led_tick;

    const bool active = program_led_status == RUNNING
        ? program_led_tick % 8u == 0u
        : program_led_tick % 4u < 2u;

    gpio_put(PIN_STATUS_LED, active);

    return true;
}


void sequencer_leds_init(void)
{
    for (uint i = 0; i < MAX_PULSE_CHANNELS; ++i) {
        const uint pin = PINS_OUTPUT_LEDS[i];

        gpio_init(pin);
        gpio_put(pin, 0);
        gpio_set_dir(pin, GPIO_OUT);
    }

    const uint pins[3] = {
        PIN_TRIGGER_GATE_LED,
        PIN_TRIGGER_EXT_LED,
        PIN_STATUS_LED
    };

    for (uint i = 0; i < 3; ++i) {
        gpio_init(pins[i]);
        gpio_put(pins[i], 0);
        gpio_set_dir(pins[i], GPIO_OUT);
    }
}


void sequencer_leds_output_update(
    const struct pulse_scpi_config* scpi
) {
    for (uint i = 0; i < MAX_PULSE_CHANNELS; ++i) {
        gpio_put(
            PINS_OUTPUT_LEDS[i],
            scpi[i + MAX_CLOCK_SYNCS].state
        );
    }
}


void sequencer_leds_input_update(
    const struct pulse_scpi_config* scpi
) {
    gpio_put(
        PIN_TRIGGER_GATE_LED,
        scpi[0].gate_mode != SCPI_GATE_DISABLED
    );

    gpio_put(
        PIN_TRIGGER_EXT_LED,
        scpi[0].trig_mode != SCPI_TRIGGER_DISABLED
    );
}


void sequencer_leds_program_update(
    uint32_t status
) {
    // Preserve the blink phase while the status remains unchanged.
    if (status == program_led_status)
        return;

    cancel_repeating_timer(&program_led_timer);

    program_led_status = status;
    program_led_tick = 0;

    if (status == RUNNING || status == ABORTED) {
        gpio_put(PIN_STATUS_LED, 1);

        add_repeating_timer_ms(
            -250,
            sequencer_leds_program_timer,
            NULL,
            &program_led_timer
        );
    }
    else {
        gpio_put(PIN_STATUS_LED, status == IDLE);
    }
}