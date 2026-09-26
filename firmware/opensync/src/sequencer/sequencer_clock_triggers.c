#include "sequencer_clock_triggers.h"

#include "hardware/gpio.h"

#include "sequencer/sequencer_common.h"


bool sequencer_clock_triggers_init(
    PIO pio, 
    const struct pulse_scpi_config* scpi
) {
    if (scpi == NULL || 
        (pio != pio0 && pio != pio1 && pio != pio2) ||
        scpi -> trig_mode > SCPI_TRIGGER_ENABLED || 
        scpi -> trig_edge > SCPI_TRIGGER_FALLING ||
        scpi -> gate_mode > SCPI_GATE_CHANNEL || 
        scpi -> gate_level > SCPI_GATE_HIGH
    ) {
        return false;
    }

    const uint trigger = PINS_TRIGGER_EXTERNAL[scpi -> trig_mode ? scpi -> trig_edge + 1u : 0u];
    const uint gate = PINS_TRIGGER_GATE[scpi -> gate_mode ? scpi -> gate_level + 1u : 0u];
    const uint disabled = PINS_TRIGGER_EXTERNAL[0];

    // GPIO 4 is the shared inactive source. Initialize it only once.
    gpio_init(disabled);
    gpio_set_dir(disabled, GPIO_IN);
    gpio_pull_down(disabled);

    if (trigger != disabled) {
        gpio_init(trigger);
        gpio_set_dir(trigger, GPIO_IN);
        gpio_disable_pulls(trigger);
    }
    
    // CHANNEL mode can use either polarity on different output channels.
    for (uint i = 1; i < 3; ++i) {
        const uint pin = PINS_TRIGGER_GATE[i];

        if (gate == pin || scpi -> gate_mode == SCPI_GATE_CHANNEL) {
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
        }
    }

    // Each SM helper sets only its own output's level and direction.
    pio_gpio_init(pio, PIN_CLOCK_T0);
    pio_gpio_init(pio, PIN_CLOCK_INTERNAL);

    return true;
}

void sequencer_clock_triggers_deinit(
    const struct pulse_scpi_config* scpi
) {
    if (scpi == NULL || scpi -> trig_mode > SCPI_TRIGGER_ENABLED ||
        scpi -> trig_edge > SCPI_TRIGGER_FALLING || scpi -> gate_mode > SCPI_GATE_CHANNEL ||
        scpi -> gate_level > SCPI_GATE_HIGH
    ) {
        return;
    }

    const uint trigger = PINS_TRIGGER_EXTERNAL[scpi -> trig_mode ? scpi -> trig_edge + 1u : 0u];
    const uint gate = PINS_TRIGGER_GATE[scpi -> gate_mode ? scpi -> gate_level + 1u : 0u];
    const uint disabled = PINS_TRIGGER_EXTERNAL[0];

    gpio_deinit(PIN_CLOCK_T0);
    gpio_deinit(PIN_CLOCK_INTERNAL);

    if (trigger != disabled)
        gpio_deinit(trigger);

    for (uint i = 1; i < 3; ++i) {
        const uint pin = PINS_TRIGGER_GATE[i];

        if (gate == pin || scpi -> gate_mode == SCPI_GATE_CHANNEL) 
            gpio_deinit(pin);
    }

    gpio_deinit(disabled);
}
