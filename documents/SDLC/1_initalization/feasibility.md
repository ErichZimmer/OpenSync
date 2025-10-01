<div align="center">

# On the Feasibility of an Open-Source 12-Channel Synchronizer



Erich Zimmer

On the behalf of the\
*OpenPIV Consortium*

5/29/2025
</div>

## Version History
| Date | Version | Change | 
| --- | --- | --- |
| 5/29/2025 | 0.1.0 | Intitial publishment. |
| 5/30/2025 | 0.2.0 | Elaboration on proposed solution. |
| 6/8/2025 | 0.3.0 | Updated design due to RP2350 limitations. |
| 6/10/2025 | 0.4.0 | Finished the solutions section. |
| 9/2/2025 | 0.4.1 | Deadline exttension |
 
## Table of Contents
 1. Introduction
 2. Project Overview
 3. Solutions
 5. References

## 1. Introduction

### 1.1 Briefing
Digital pulse generating devices are often deployed in laboratory environments to control various instrumental equipment. These devices allow for the automation of experimental test setups when performing measurements. A key feature of these devices is the ability to synchronize multiple devices in order to complete a common goal. For instance, particle image velocimetry (PIV) is a non-intrusive optical technique for directly measuring fluid flow structures and often requires a minimum of five channels to properly synchronize all devices during data acquisition. Additionally, a high degree of accuracy and precision is necessary for a PIV system to obtain reliable measurements. This created a demand for high accuracy/precision digital pulse generators for the purpose of specialized niche laboratory settings. This demand has mostly been filled via commercial offerings that provide ample performance and flexibility to perform all but the most complex of experiments. Nonetheless, commercial offerings for these devices can be exceedingly expensive which may contribute to the overall issue of a lack of adoption of PIV systems due to insufficient funding. As a first effort to lower the financial burden of acquiring PIV hardware, the feasibility of designing and constructing an open-source 16-channel synchronizer with advanced capabilities will be discussed.

### 1.2 Purpose
Commercial digital pulse generators, also known as synchronizers, can cost many thousands of dollars. For instance, one commercial synchronizer had a cost-of-ownership of around \$9,000 USD which makes it financially infeasible to obtain for institutions and organizations that lack funding (I. Nepomnyi, personal communication, May 18, 2025). This reflects the overall problem where even a simple PIV system can easily exceed over \$100,000 when purchasing hardware from a commercial provider (Ring & Lemley, 2020). One of the great inspiration of this project were the low-cost and highly performant PIV equipment supplied by the commercial providers Optolution and Microvec (Optolution GmbH, 2025; Microvec PTE LTD, 2025). These commercial providers made significant strides towards lowering the financial burden of obtaining a PIV system. However, the cost-of-ownership may still be too burdensome for some. Kojo et al. (2024) stated that many developing and poverty countries have significant difficulties in obtaining PIV systems due to a rather stringent financial constraint. As such, the designed PIV hardware in this effort to lower the financial burden of acquiring PIV systems should be as low-cost as possible while retaining several advanced features commonly seen in commercial devices.

### 1.3 Objectives
The primary objective of this project is to explore the development of low-cost hardware solutions for high performant, research-grade PIV systems. Given the significant financial barriers associated with commercial devices, there is a pressing need for affordable alternatives that can be utilized by a broad range of institutions and organizations. As a first effort of lowering the cost of PIV hardware, this project specifically would focus primarily on synchronization devices (e.g., digital pulse generators). It is hoped that the end result of this on-going effort would result in a very affordable, open-source, and performant synchronizer capable of being applied in a very broad range of experiments.

### 1.4 Scope
The scope of this project is very simple: design an easily manufacturable and performant synchronizer that remains as low-cost as possible. All other PIV hardware and considerations remain outside the scope of this particular project. The emphasis on affordability is of upmost importance as it is the central reason this effort is being made in the first place. By concentrating all efforts on synchronizing hardware, the feasibility of expanding this project to other PIV hardware becomes more readily obtainable.

## 2. Project Overview

### 2.1 Operational Details
This project aimed towards designing and constructing a low-cost synchronizer for applications of fluid flow diagnostics is privately funded by Erich Zimmer (the author of this project). Nearly all responsibilities for the design, construction, and validation of any devices produced during this project would fall on this person. This project has a finalized budget of ~$600 USD (2025) which also includes the costs of obtaining equipment for the purpose of electrical engineering (e.g., soldering station, osciliscope, etc). Finally, the project is in its very early developmental phase and is expected to remain so until the project officially starts.

### 2.2 Project Schedule

| Date | Event |
| --- | --- |
| 09/02/2025 | Initial hardware engineering project starting point. |
| 09/30/2025 | Initial firmware design created. |
| 10/14/2025 | Initial circuitry and CAD models designed |
| 10/21/2025 | Preliminary report on results of project. |
| 10/28/2025 | Second report on results of project. |
| 11/4/2025 | First presentation on the results of the project. |
| 11/11/2025 | Second presentation on the final synchronizer model. |
| 11/18/2025 | Final presentation and paper on the designed synchronizer. |

## 3. Solutions

### 3.1 Previous Solutions
Of all the published literature on the topics of lowering the prohibitive costs PIV systems, most seldomly discuss alternatives to expensive commercial PIV hardware. For example, a tomo-PIV experiment involving high-powered light emitting diodes (LEDs) and four smartphones utilized a Stanford Research DG645 pulse generator, by far the most expensive component in that whole experiment (Aguirre-Pablo et al., 2017). Kojo et al. (2024) replaced the use of an expensive commercial synchronizer device with an Arduino Nano in their low-cost PIV setup. This effectively allows for a PIV system to be controlled in a somewhat traditional way using external triggers and pulsed light sources. However, their system could only pulse a repetition frequencies that were hardcoded into the firmware of the Arduino Nano. This makes this system mostly unsuitable for the requirements of this project.

A further attempt by the author was made based on the arbitrary pulse generator principle as described in Radim and Miroslav (2018) using a Teensy 4.1 development board. The Teensy 4.1 development board utilized an Arm Cortex m7 microcontroller which runs at a baseline clock frequency of 600 MHz. With such as high clock frequency, pulse timing would have an excellent resolution of less than two nanoseconds per clock cycle. This makes this microcontroller a good candidate for implementing a synchronizing device. Using simple logic and the concept of bit-banging, a synchronizing device with up to 32 outputs and one external trigger input can be readily idealized (Radim & Miroslav, 2018). However, such logic could be quite restrictive depending on the demands of the laboratory experiment. For instance, lets say that the synchronizer device needs to keep track of two external events. Since microcontrollers are mostly serial devices, multiple pulse configurations cannot co-exist at the same exact time. This means that additional logic must be implemented such that it would check for both events and execute them on after another (not in parallel). Due to such limitations, the synchronizer device would remain applicable to mostly simple designs and experiments. Even with the existence of these limitations, the vast majority of PIV experiments do not require advance synchronizing techniques, so such limitations could be deemed inconsequential. As a proof-of-concept, the author of this study implemented a 16-channel synchronizer that has a number of features that makes it applicable for use in studies utilizing PIV. For instance, the synchronizer device can accept a five volt transistor-transistor logic (TTL) input as an external trigger of which the device can behave in different pre-configured ways based on the trigger signal. These pre-configured behaviors include external trigger sync, number of external triggers to ignore, delay between the external trigger and beginning of the pulse sequence, and enabling the pulse sequence on TTL high or low signals. Nonetheless, the development of this synchronizer design was haulted in favor of the proposed solution which promises significantly more advanced features.

By far the most promising design for engineering a synchronizing device was published in an article by Starkey et al. (2024). Their system utilizes two microcontrollers: one focused on producing clock frequencies and another on arbitrary pulse generation. The microcontroller focused on producing clock frequencies (also known as a pseudoclock), is used to control how the arbitrary pulse generator behaves through externally triggering the second microcontroller. This unique combination of microcontrollers allows for one to effectively control nearly all laboratory equipment in a precise and low-cost manner. One of the most important reasons why this solution is so effective is because of the microcontrollers utilized in their system, which will be discussed further in the proposed solution. Without this microcontroller, features that would otherwise be readily implemented in simple programming languages would require more complex devices and possibly the use of hardware-design languages (HDL).

### 3.2 Proposed Solution
The proposed solution is based on microcontroller architecture and utilizes a somewhat unique microcontroller. The RP2350 microcontroller produced by the Raspberry Pi Foundation is a dual-core chip that contains a remarkable 12 state machines split between three programmable input/output (PIO) blocks. The unique hardware of this microcontroller makes it particularly fit for the implementation of a multi-channel synchronizing device due to each state machine having the ability to run programs in parallel and asynchronous from the main system. This allows for complex programs that usually require complex programmable logic devices (CPLDs) or similar devices to be readily idealized in microcontroller architecture. Starkey et al. (2024) were among the first authors to publish a digital delay/pulse generator utilizing this microcontroller chip and obtained great success. Their system is based on a dual-architecture logic where one microcontroller produces clock signals used for synchronizing devices and another microcontroller acts as an arbitrary pulse generator. The system proposed heroin closely models this system as it provides the foundation for producing high-performance microcontroller-based synchronizing devices.

The ideal synchronizer should have enough flexibility and output channels to accommodate most users' needs while remaining simple to operate. The vast majority of PIV experiments utilize an actively q-switched dual-oscillator neodymium-doped yttrium aluminum garnet (Nd:YAG) laser and a singular camera (Raffel et al., 2018). Dual oscillator PIV lasers require a minimum of four output channels to control the laser pump and active q-switch for each laser oscillator. Additionally, many studies make use of stereoscopic PIV which can readily extend single-view planar PIV by using a second camera and simple triangulation algorithms to reconstruct the velocities in the third dimension (Raffel et al., 2018). For more advanced techniques such as tomographic-PIV (tomo-PIV), at least four cameras are necessary to obtain reliable volumetric data from an experimental volume. This would require a minimum of eight output channels in order to allow for the complete control over both the cameras and laser.

Since most PIV experiments do not require greater than 12 independent outputs to synchronize an experiment, the proposed solution implements 12 output channels that can be mapped to an arbitrary pulse generator. However, synchronizers that are to be used in PIV experiments require more complexity than a mere arbitrary pulse generator. For instance, some experiments may need to be phase synchronized in order to obtain any meaningful data. This necessitates the inclusion of an external trigger to the synchronizing device which would control the execution of the arbitrary pulse generator. While this solution is simple enough to function as a synchronizer for most PIV experiments, more complex PIV experiments require further features in order to make an open-source synchronizer feasible. Based on the results from Starkey et al. (2024), it would be advantageous to include both an internal synchronizing clock and arbitrary pulse generating function inside a singular device. The combination of these two features could allow for rather complex pulse sequences and timing schemes to be readily implemented without the need of relying on commercial hardware. This in turn could make the open-source synchronizer a viable alternative to its commercial counterparts when limited funding needs to be taken into account.

While the firmware design of the synchronizer is quite important, it is important to consider the availability of hardware components that would be used in the synchronizer device. All electrical components should have an active product status and be readily available with minimal costs and lead-times. Additionally, all parts that need to be manufactured in-house must remain simple enough to require minimal expertise to do so. In this particular case, an emphasis is placed on additive manufacturing technology (e.g., 3D printing) for hardware parts such as device enclosures and face plates. This would hopefully allow for synchronizer devices to be easily constructed in-house that are both professional in appearance and minimal in cost-of-ownership.

### 3.3 Alternative Solutions
A final possible solution is based on field-programmable gate array (FPGA) architecture and would possibly allow for great accuracy/precision and timing. FPGAs require the use of HDLs such as SystemVerilog to "program" the chip along with additional measures to ensure that the FPGA is behaving as intended. This is because FPGAs are parallel devices by default, which contrasts to microcontrollers whom are almost always serial in design. FPGAs can be networked with other devices to drastically increase performance and reliability. For instance, Chen et al. (2021) implemented a digital pulse generator using FPGA architecture with timing jitters below 0.5 nanoseconds. With a timing resolution of around 16 picoseconds as reported by Chen et al. (2021), this system becomes extremely attractive for the implmentation of a low-cost and high performance synchronizer device. However, this solution was not investigated further due to the aforementioned proposed solution and the author's lack of expertise in programming FPGAs using HDLs.

## 4. References
 1. Aguirre-Pablo, A. A., Alarfaj, M. K., Li, E.-P., Hernández-Sánchez, J. F., & Thoroddsen, S. T. (2017). Tomographic Particle Image Velocimetry using Smartphones and Colored Shadows. 7(1). https://doi.org/10.1038/s41598-017-03722-9
 2. Chen, Z., Wang, X., Zhou, Z., Moro, R., & Ma, L. (2021). A simple Field Programmable Gate Array (FPGA) based high precision low-jitter delay generator. Review of Scientific Instruments, 92(2). https://doi.org/10.1063/5.0030341
 3. Kojo, F., Paul, J., & Beem, H. R. (2024). Design and characterization of a low-cost particle image velocimetry system. HardwareX, 19(e00563), e00563–e00563. https://doi.org/10.1016/j.ohx.2024.e00563
 4. Microvec PTE LTD. (2025). MicroVec 2D PIV System Flow field diagnostics made easier. https://piv.com.sg/piv-products/2d-piv-system/
 5. Optolution GmbH. (2025). Optolution, Laser diode PIV system (air). https://optolution.com/en/products/particle-image-velocimetry-piv/laser-diode-piv-system-air/
 6. Raffel, M., Willert, C. E., Fulvio Scarano, KählerC. J., Wereley, S. T., & Jürgen Kompenhans. (2018). Particle Image Velocimetry. Cham Springer International Publishing
 7. Radim Hošák, & Miroslav Ježek. (2018). Arbitrary digital pulse sequence generator with delay-loop timing. Review of Scientific Instruments, 89(4). https://doi.org/10.1063/1.5019685
 8. Ring, B. P., & Lemley, E. C. (2020). Design and Implementation of a Low-cost Particle Image Velocimetry System for Undergraduate Research and Education. Papers on Engineering Education Repository (American Society for Engineering Education). https://doi.org/10.18260/1-2--20256
 9. RP2350 Datasheet, A Microcontroller by Raspberry Pi. (2023). Raspberry Pi Ltd. https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
 10. Starkey, P. T., Turnbaugh, C., Miller, P., LeBlanc, K.-J., & Meyer, D. H. (2024). Experimental timing and control using microcontrollers. Review of Scientific Instruments, 95(10). https://doi.org/10.1063/5.0225550

