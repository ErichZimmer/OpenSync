# OpenSync
An open source synchronizer for the velocimetry of fluids using a Raspberry Pi Pico 2 microcontroller.

# NOTICE
This is for a future project for an undergraduate course (that will be completed 2 semesters from now). The course is over embedded systems and programming design, and I'll think it will greatly enhance the code for the synchronizer. Currently, this repository is going to be left blank until I obtain a 2 or 4 channel >100MHz oscilloscope. Also, I decided to shift towards using a heavily modified version of the PrawnDO firmware (reference 3) due to its deemed superiority compared to what I came up with (which was something quite similar to be honest, but I wanted to use their USB serial interface code).

## The Plan
This idea was inspired by the use of an Arduino Uno to send TTL pulses to two Arducam ~1 MP monochrome global shutter cameras on one of my previouse experiments. However, that implementation used a simple loop that was hard-coded. To increase flexibility, a serial I/O interface should be created so that the user can change the parameters of the synchronizer while the device is on (not operating, though, as that could cause damage to lab equipment and maybe even the synchronizer itself). Due to the dynamic nature of the serial interface, optimization from hard-coding would be lost which could cause a loss in temporal resolution and accuracy. However, very efficient execution of instructions could be done using programmable input/output (PIO) on a RP2350-based microcontroller (e.g., Pico 2). The RP2350 could be used as an effective 8-channel arbitrary pulse generator with jitter as low as one clock cycle (10ns to 4ns). The dual-core nature of the RP2350 allows for one core to be dedicated to IO and another to the serial interface, thus alleviating the aforementioned issues with Arduino microcontrollers. The synchronizer is programmed using an 8 bit word (one bit for each channel) and a 32 bit word (delay to next instruction). The RP2350 supports a few thousand instructions, so great flexibility could be achieved. An external trigger (optional) could be used to execute the pulse sequence along with internal timing based on repitition in Hertz. The entire system can be 3D printed and soldered in-house in as little as an afternoon. This system would also allow for the use of up to 4 cameras in addition to a dual-head YAG/YLF laser.

## Design
OpenSync is a simple and low-cost synchronizer based on microcontroller technology. Due to the intrinsic nature of microcontrollers compared to more advanced devices (e.g., field programmable gate arrays), most complex features that are typical of commercial propietary devices such as gating, external trigger manipulation, etc are not implemented. However, OpenSync remains sufficiently flexible and provides enough support for most users' needs when performing a PIV experiment. This is because OpenSync is an arbitrary pulse generator under-the-hood which allows for rather complex pulse sequences to be synthesized through simple output port manipulation. While this implementation for a synchronizer may be quite restrictive at times, it is what makes OpenSync so simple!

## Specs
 - **Main PLL Frequency**: overclocked to 200 MHz
 - **Output Channels**: 8 (supports up to 24)
 - **Input Channel(s)**: 1
 - **I/O Channel Voltage**: 5V
 - **I/O Resolution**: 1 clock cycle (5ns * clock divider)
 - **Min Pulse Width**: 5 clock cycles (25ns * clock divider)
 - **Max Pulse Width**: 2^32 - 1 cycles (~21.47s * clock divider)
 - **Ext. Trigger Min Pulse Width**: 5 clock cycles (25ns * clock divider)
 - **Ext. Trigger Jitter**: 2 clock cycles (10ns * clock divider)
 - **Max Pulse Sequence Length**: 60,000 instructions
 - **Pulse Sequence Repetition Jitter**: 8 clock cycles (40ns)
 - All powered via USB! (for now)

## Documentation
Complete instructions to construct and use OpenSync are provided in [imaginary link to notebook-based rtd website].

## Quick Start

### Compilation
TBD

### Flashing
TBD

### Self Test
TBD

## Context
https://groups.google.com/g/openpiv-users/c/xi7qt28IGEE

## Acknowledgments
Will be determined after the first milestone of this project.

## References
 1. Radim Hošák, & Miroslav Ježek. (2018). Arbitrary digital pulse sequence generator with delay-loop timing. Review of Scientific Instruments, 89(4). https://doi.org/10.1063/1.5019685
 2. Scallon, A. (2017, June 18). Arduino TTL Pulse Generator and Controller. Optogenetics and Neural Engineering Core. https://optogeneticsandneuralengineeringcore.gitlab.io/ONECoreSite/projects/Arduino%20TTL%20Pulse%20Generator%20and%20Controller/
 3. Starkey, P. T., Turnbaugh, C., Miller, P., LeBlanc, K.-J., & Meyer, D. H. (2024). Experimental timing and control using microcontrollers. Review of Scientific Instruments, 95(10). https://doi.org/10.1063/5.0225550
    


