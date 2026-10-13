#include "sequencer_clock_triggers.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "sequencer/sequencer_common.h"
#include "structs/pulse_scpi_config.h"


bool sequencer_clock_triggers_init(
    PIO pio, 
    const struct pulse_scpi_config* scpi,
    bool triggered
) {
    if (scpi == NULL || 
        scpi -> trig_mode >= SCPI_TRIGGER_MODE_END || 
        scpi -> trig_edge >= SCPI_TRIGGER_EDGE_END ||
        scpi -> gate_mode >= SCPI_GATE_MODE_END || 
        scpi -> gate_level >= SCPI_GATE_LEVEL_END
    ) {
        return false;
    }

    const uint trigger = PINS_TRIGGER_EXTERNAL[triggered ? scpi -> trig_edge + 1u : 0u];
    const uint gate = PINS_TRIGGER_GATE[scpi -> gate_mode ? scpi -> gate_level + 1u : 0u];
    const uint pin_disabled = PINS_TRIGGER_GATE[0];
    const uint32_t INVERSE_LOGIC_INDEX = 2;

    // GPIO 4 is the shared inactive source. Initialize it only once.
    gpio_init(pin_disabled);
    gpio_set_dir(pin_disabled, GPIO_IN);

    // Now init the rising/falling trigger pins
    for (uint i = 1; i < 3; ++i) {
        const uint pin = PINS_TRIGGER_EXTERNAL[i];

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    // Now init the gate high/low pins
    for (uint i = 1; i < 3; ++i) {
        const uint pin = PINS_TRIGGER_GATE[i];

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    // Now set inverse logics for the selected pins.
    gpio_set_inover(
        PINS_TRIGGER_EXTERNAL[INVERSE_LOGIC_INDEX],
        GPIO_OVERRIDE_INVERT
    );
    gpio_set_inover(
        PINS_TRIGGER_GATE[INVERSE_LOGIC_INDEX], 
        GPIO_OVERRIDE_INVERT
    );

    // Each SM helper sets only its own output's level and direction.
    pio_gpio_init(pio, PIN_CLOCK_T0);
    pio_gpio_init(pio, PIN_CLOCK_INTERNAL);

    return true;
}

void sequencer_clock_triggers_deinit(
    const struct pulse_scpi_config* scpi,
    bool triggered
) {
    if (scpi == NULL ||
        scpi -> trig_mode >= SCPI_TRIGGER_MODE_END || 
        scpi -> trig_edge >= SCPI_TRIGGER_EDGE_END ||
        scpi -> gate_mode >= SCPI_GATE_MODE_END || 
        scpi -> gate_level >= SCPI_GATE_LEVEL_END
    ) {
        return;
    }

    const uint trigger = PINS_TRIGGER_EXTERNAL[triggered ? scpi -> trig_edge + 1u : 0u];
    const uint gate = PINS_TRIGGER_GATE[scpi -> gate_mode ? scpi -> gate_level + 1u : 0u];
    const uint disabled = PINS_TRIGGER_GATE[0];

    gpio_deinit(PIN_CLOCK_T0);
    gpio_deinit(PIN_CLOCK_INTERNAL);
    gpio_deinit(disabled);

    for (uint i = 1; i < 3; ++i)
        gpio_deinit(PINS_TRIGGER_EXTERNAL[i]);

    for (uint i = 1; i < 3; ++i)
        gpio_deinit(PINS_TRIGGER_GATE[i]);
}


// Call on core_2 after arming, using the selected trigger pin and mode.
void sequencer_trigger_manual(
    uint pin_trigger,
    bool triggered
) {
    gpio_set_inover(pin_trigger, GPIO_OVERRIDE_LOW);

    sleep_us(250);

    gpio_set_inover(pin_trigger, GPIO_OVERRIDE_HIGH);

    sleep_us(250);

    gpio_set_inover(
        pin_trigger,
        triggered ? GPIO_OVERRIDE_NORMAL : GPIO_OVERRIDE_LOW
    );
}
