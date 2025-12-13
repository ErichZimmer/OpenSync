# OpenSync Arbitrary Pulse Generator
An open source synchronizer for the velocimetry of fluids using a Raspberry Pi microcontroller.

## GENERAL
 - **Output Channels**: 8
 - **Input Channels**: 1
 - **I/O Channel Voltage**: 5V @ 50 Ohm
 - **Max Clock Divider**: 65500
 - **Communications**: USB 2.0 OTG (On-The-Go)
 
## INTERNAL TIMING GENERATOR
 - **Oscillator**: 12 MHz, 30 ppm crystal oscillator
 - **PLL Frequency**: 250 MHz (4 ns resolution)
 - **PLL Jitter**:  typically < 0.1 ns
 - **Internal Sync Clocks**: 3
 - **Repetition Rate**: 0.059 Hz to 20 MHz (~9e-7 Hz to ~305 Hz with max clock divider)
 - **Pulse Control Modes**: freerun, external trigger (rising edge)
 - **Ext. Trigger Min Pulse Width**: 8 ns * clock divider
 - **Ext. Trigger Delay**: 12 ns * clock divider

## CHANNEL TIMING GENERATOR
 - **I/O Resolution**: 4 ns * clock divider
 - **Pulse Width**: 20 ns to ~17 s (~1.3 ms to ~311 h with max clock divider)
 - **Max Pulse Sequence Length**: 32 instruction pairs
 - **Pulse Sequence Repetition**: Up to 500,000 iterations
 - **Single-Channel Jitter**:  typically < 0.1 ns
 - **Inter-Channel Jitter**:  typically < 0.3 ns
 - **Rise/Fall Time**: typically < 4 ns