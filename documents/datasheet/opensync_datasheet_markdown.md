# OpenSync Arbitrary Pulse Generator
An open source synchronizer for the velocimetry of fluids using a Raspberry Pi microcontroller.

## GENERAL
 - **Output Channels**: 8
 - **Input Channels**: 1
 - **I/O Channel Voltage**: 5V @ 50 ohm
 - **Output Current**: 50 mA into 50 ohm, 5 mA into 1k ohm
 - **Output Voltage**: 2.5 V into 50 ohm, 5 V into high impedance
 - **Clock Resolutions**: 1, 2, 5, 25, 250
 - **Communications**: USB 2.0 OTG (On-The-Go)
 
## INTERNAL TIMING GENERATOR
 - **Oscillator**: 12 MHz, 30 ppm crystal oscillator
 - **PLL Frequency**: 250 MHz (4 ns resolution)
 - **PLL Jitter**:  typically < 0.1 ns
 - **Internal Sync Clocks**: 3
 - **Repetition Rate**: 0.06 Hz to 1.9 MHz (0.0004 Hz to 7 kHz with max clock divider)
 - **Pulse Control Modes**: freerun, external trigger (rising edge)
 - **Ext. Trigger Min Pulse Width**: 8 ns * clock divider
 - **Ext. Trigger Delay**: 16 ns * clock divider

## CHANNEL TIMING GENERATOR
 - **I/O Resolution**: 4 ns * clock divider (e.g., 4 ns, 8 ns, 20 ns, 100 ns, 1 us resolutions)
 - **Pulse Width**: 20 ns to 15 s (0.5 ms to 4,000 s with max clock divider)
 - **Max Pulse Sequence Length**: 16 instruction pairs
 - **Max Pulse Sequence Repetition**: 500,000 iterations
 - **Single-Channel Jitter**:  typically < 0.1 ns
 - **Inter-Channel Jitter**:  typically < 0.3 ns
 - **Rise/Fall Time**: typically < 4 ns