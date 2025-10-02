#include "overclock.h"

// TODO: Add compile-time ocnventions for clock speed? (e.g., 4 ns .. 2.5 ns resolution)
// Set the system clock speed
void overlcok_system_set()
{
    // Increase voltage to support overclock
    vreg_set_voltage(VREG_VOLTAGE_1_05);

    // Set PLL frequency to 250 MHz
    set_sys_clock_khz(250000, true);
}