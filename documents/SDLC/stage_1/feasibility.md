<div align="center">

# On the Feasibility of an Open-Source 16-Channel Synchronizer



Erich Zimmer

On the behalf of the\
*OpenPIV Consortium*

5/29/2025
</div>

## Version History
| Date | Version | Change | 
| --- | --- | --- |
| 5/29/2025 | 0.1 | Intitial publishment. |

## Table of Contents
 1. Introduction
 2. Project Overview
 3. Solutions
 5. References

## 1. Introduction

### 1.1 Briefing
Digital pulse generating devices are often deployed in laboratory environments to control various instrumental equipment. These devices allow for the automation of experimental test setups when performing measurements. A key feature of these devices is the ability to synchronize multiple devices in order to complete a common goal. For instance, particle image velocimetry (PIV) is a non-intrusive optical technique for directly measuring fluid flow structures and often requires a minimum of five channels to properly synchronize all devices during data aquisition. Additionally, a high degree of accuracy and precision is necessary for a PIV system to obtain reliable measurements. This created a demand for high accuracy/precision digital pulse generators for the purpose of specialized niche laboratory settings. This demand has mostly been filled via commercial offerings that provide ample performance and flexibility to perform all but the most complex of experiments. Nonetheless, commercial offerings for these devices can be exceedingly expensive which may contribute to the overall issue of a lack of adoption of PIV systems due to insufficient funding. As a first effort to lower the financial burden of aquiring PIV hardware, the feasibility of designing and constructing an open-source 16-channel synchronizer with advanced capabilities will be discussed.

### 1.2 Purpose
Commercial digital pulse generators, also known as synchronizers, can cost many thousands of dollars. For instance, one commercial synchronizer had a cost-of-ownership of around \$9,000 USD which makes it financially infeasible to obtain for institutions and organizations that lack funding (I. Nepomnyi, personal communication, May 18, 2025). This reflects the overall problem where even a simple PIV system can easily exceed over \$100,000 when purchasing hardware from a commercial provider (Ring & Lemley, 2020). One of the great inspiration of this project were the low-cost and highly performant PIV equipment supplied by the commercial providers Optolution and Microvec (Optolution GmbH, 2025; Microvec PTE LTD, 2025). These commercial providers made significant strides towards lowering the financial burden of obtaining a PIV system. However, the cost-of-ownership may still be too burdensome for some. Kojo et al. (2024) stated that many developing and poverty countries have significant difficulties in obtaining PIV systems due to a rather significant financial constraint. As such, the designed PIV hardware in this effort to lower the finanical burden of aquiring PIV systems should be as low-cost as possible while retaining several advanced features commonly seen in commercial devices.

### 1.3 Objectives
The primary objective of this project is to explore the development of low-cost hardware solutions for high performant, research-grade PIV systems. Given the significant financial barriers associated with commercial devices, there is a pressing need for affordable alternatives that can be utilized by a broad range of institutions and organizations. As a first effort of lowering the cost of PIV hardware, this project specifically would focus primarily on synchronization devices (e.g., digital pulse generators). It is hoped that the end result of this on-going effort would result in a very affordable, open-source, and performant synchronizers capable of being applied in a very broad range of experiments.

### 1.4 Scope
The scope of this project is very simple: design an easily manufacturable and performant synchronizer that remains as low-cost as possible. All other PIV hardware and considerations remain outside the scope of this particular project. The emphasis on affordability is of upmost importance as it is the central reason this effort is being made in the first place. By concentrating all efforts on synchronizing hardware, the feasibility of expanding this project to other PIV hardware becomes more readily obtainable.

## 2. Project Overview

### 2.1 Operational Details
This project aimed towards designing and constructing a low-cost synchronizer for applications of fluid flow diagnostics is privately funded by Erich Zimmer (the author of this project). Nearly all resposibilities for the design, construction, and validation of any devices produced during this project would fall on this person. Finally, the project is in its early developmental phase.

### 2.2 Project Schedule

| Date | Event |
| --- | --- |
| 09/02/2025 | Initial hardware engineering project starting point. |
| 09/09/2025 | Initial circuitry and firmware design created. |
| 09/16/2025 | Preliminary report on results of project. |
| 09/23/2025 | Second report on results of project. |
| 09/30/2025 | First presentation on the results of the project. |
| 10/07/2025 | Second presentation on the final synchronizer model. |
| 10/14/2025 | Final presentation and paper on the designed synchronizer. |

## 3. Solutions

### 3.1 Previous Solution
Intentionally left blank.

### 3.2 Proposed Solution
The proposed solution is based on microcontroller architecture and utilizes a somewhat unique microcontroller. The RP2350 microcontroller produced by the Raspberry Pi Foundation is a dual-core chip that contains a remarkable 12 state machines split between three programmable input/output (PIO) blocks. The unique hardware of this microcontroller makes it particularly fit for the implementation of a multi-channel synchronizing device due to each state machine having the ability to run programs in parallel and asychronouse from the main system. This allows for complex programs that usually require complex programmable logic devices (CPLDs) or similar devices to be readily idealized in microcontroller architecture. Starkey et al. (2024) was among the first authors to publish a digital delay/pulse generator utilizing this microcontroller chip and obtained great success. Their system is based on a dual-architecture logic where one microcontroller produces clock signals used for synchronizing devices and another microcontroller acts as an arbitrary pulse generator. The system proposed heorin closely models this system as it provides the foundation for producing high-performance microcontroller-based synchronizing devices. Beyond firmware, a 3D-printable housing would be designed that encloses the hardware in a professional looking enclosure. Finally, additional electrical components would be housed inside the 3D-printed enclosure which would help fascilitate the operation and control of laboratory equipment using microcontroller architecture.

### 3.3 Alternative Solutions
Intentionally left blank.

## 4. References
 1. Kojo, F., Paul, J., & Beem, H. R. (2024). Design and characterization of a low-cost particle image velocimetry system. HardwareX, 19(e00563), e00563–e00563. https://doi.org/10.1016/j.ohx.2024.e00563
 2. Microvec PTE LTD. (2025). MicroVec 2D PIV System Flow field diagnostics made easier. https://piv.com.sg/piv-products/2d-piv-system/
 3. Optolution GmbH. (2025). Optolution, Laser diode PIV system (air). https://optolution.com/en/products/particle-image-velocimetry-piv/laser-diode-piv-system-air/
 4. Ring, B. P., & Lemley, E. C. (2020). Design and Implementation of a Low-cost Particle Image Velocimetry System for Undergraduate Research and Education. Papers on Engineering Education Repository (American Society for Engineering Education). https://doi.org/10.18260/1-2--20256
 5. RP2350 Datasheet, A Microcontroller by Raspberry Pi. (2023). Raspberry Pi Ltd. https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
 6. Starkey, P. T., Turnbaugh, C., Miller, P., LeBlanc, K.-J., & Meyer, D. H. (2024). Experimental timing and control using microcontrollers. Review of Scientific Instruments, 95(10). https://doi.org/10.1063/5.0225550
