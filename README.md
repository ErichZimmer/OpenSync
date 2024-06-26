# OpenSync
An open source synchronizer for velocimetry of fluids using an Arduino Due microcontroller.

# NOTICE
This is for a future project for an undergraduate course (that will be completed 5 semesters from now). The course is over embedded systems and programming design, and I'll think it will greatly enhance the code for the synchronizer. Currently, this repository is going to be left blank until I obtain an Arduino Due, Intel FPGA with secondary 8 bit oscillator, and possibbly a 2 or 4 channel oscilloscope.

## The Plan
This idea was inspired by the use of an Arduino Uno to send TTL pulses to two Arducam ~1 MP monochrome global shutter cameras on one of my previouse experiments. However, that implementation used a simple loop that was hard-coded. To increase flexibility, a serial I/O interface should be created so that the user can change the parameters of the synchronizer while the device is on (not operating, though, as that could cause damage to lab equipment and maybe even the synchronizer itself). Due to the dynamic nature of the serial interface, optimization from hard-coding would be lost which could cause a loss in temporal resolution and accuracy. To minimize this possible predicament, I plan on going down to AVR level and using AVR-GCC for compilation. Additionally, I plan on making a 4 chanel synchronizer using an Intel FPGA with a secondary 8 bit clock for jitter compensation. Thoeretically, the FPGA synchronizer should be able to obtain sub-nanosecond jitter and resolution which makes it attractive for PIV applications. However, the FPGA may need a CPU for serial interfacing and other utilities since it is difficult to design everything in hardware design language (especially for VHDL).

## Context
https://groups.google.com/g/openpiv-users/c/xi7qt28IGEE
