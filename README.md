# OpenSync
An open source synchronizer for the velocimetry of fluids using a Raspberry Pi Pico 2 microcontroller.

# NOTICE
This is for a future project for an undergraduate course (that will be completed 2 semesters from now). The course is over embedded systems and programming design, and I'll think it will greatly enhance the code for the synchronizer. Currently, this repository is going to be left blank until I obtain a 2 or 4 channel >100MHz oscilloscope.

## The Plan
This idea was inspired by the use of an Arduino Uno to send TTL pulses to two Arducam ~1 MP monochrome global shutter cameras on one of my previouse experiments. However, that implementation used a simple loop that was hard-coded. To increase flexibility, a serial I/O interface should be created so that the user can change the parameters of the synchronizer while the device is on (not operating, though, as that could cause damage to lab equipment and maybe even the synchronizer itself). Due to the dynamic nature of the serial interface, optimization from hard-coding would be lost which could cause a loss in temporal resolution and accuracy. However, very efficient execution of instructions could be done using programmable input/output (PIO) on a RP2350-based microcontroller (e.g., Pico 2). The RP2350 could be used as an effective 8-channel arbitrary pulse generator with jitter as low as one clock cycle (10ns to 4ns). The dual-core nature of the RP2350 allows for one core to be dedicated to IO and another to the serial interface, thus alleviating the aforementioned issues with Arduino microcontrollers. The synchronizer is programmed using an 8 bit word (one bit for each channel) and a 32 bit word (delay to next instruction). The RP2350 supports a few thousand instructions, so great flexibility could be achieved. An external trigger (optional) could be used to execute the pulse sequence along with internal timing based on repitition in Hertz. The entire system can be 3D printed and soldered in-house in as little as an afternoon. This system would also allow for the use of up to 4 cameras in addition to a dual-head YAG/YLF laser.



## Context
https://groups.google.com/g/openpiv-users/c/xi7qt28IGEE
