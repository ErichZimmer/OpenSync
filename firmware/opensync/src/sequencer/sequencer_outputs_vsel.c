#include "sequencer_outputs_vsel.h"

#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/types.h"

#include "sequencer/sequencer_common.h"
#include "structs/pulse_scpi_config.h"


void sequencer_vsel_init(void)
{
    for (uint i = 0; i < MAX_PULSE_CHANNELS; ++i) {
        const uint pin = PINS_OUTPUT_VSEL[i];

        gpio_init(pin);
        gpio_put(pin, 0); // Off/low
        gpio_set_dir(pin, GPIO_OUT);
    }
}


void sequencer_vsel_update(
    const struct pulse_scpi_config* scpi
) {
    for (uint i = 0; i < MAX_PULSE_CHANNELS; ++i) {
        gpio_put(
            PINS_OUTPUT_VSEL[i],
            scpi[i + MAX_CLOCK_SYNCS].level
        );
    }

    // Delay to make sure the capcitors are stabelized
    sleep_ms(500);
}