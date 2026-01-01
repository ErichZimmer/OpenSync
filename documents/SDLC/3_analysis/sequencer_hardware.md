# On the design and implementation of a Synchronizer Timing Hardware

## Version History
| Date | Version | Change | 
| --- | --- | --- |
| 10/24/2025 | 0.1.0 | Intitial publishment. |
| 12/31/2026 | 0.2.0 | Updated document. |

## Key Words
- GPIO - General Purpose Input/Output
- IC - Internal Circuit
- PCB - Printed Circuit Board


## 1. Introduction
The design and construction of the OpenSync synchronizer will consist of two parts: the PCB and its associated enclosure. The PCB contains all electronics and should ideally require minimal soldering after reception from a PCB assembly vendor. To protect the PCB from the external environment, an enclosure that can be constructed using additive manufacturing technology (e.g., 3D-printing) will also be designed. The combination of easily obtainable PCBs and enclosures is of significant importance as highlighted below.

## 2.1 Enclosure
Electronics, especially specialized laboratory equipment, are often used throughout different environments that require some degree of isolation from. For instance, dusty environments can cause shorts in exposed circuit boards. Enclosures for PCBs are often the most suitable form of isolation for PCBs due to their simplicity. In the case for a multi-channel digital delay/pulse generator, a simple enclosure to protect the housed circuitry is often enough in most scenarios. This allows for the design of an enclosure to fascilitate easy of use while providing protection for internal circuity to be mostly trivial.

Since the main PCB of the synchronizer device is a half Eurocard, standardized enclosures already exist in the European market. The materials of these encolsures range from plastic to aluminium. However, modifying these pre-existing encolusres to the needs of this project may be quite cumbersome. As such, a different approach has been taken through the use of additive manufacturing technology. Additive manufacturing has become significantly more acessibly during recent years. For instance, a vat photopolymerization printer can be aquired from several different Chinese manufactures with manufacturing volumes exceeding 200 mm x 120 mm x 200 mm for less than $300 USD. Furthermore, the precision of these conumer-grade additive manufacturing systems has increased to beyond 20 microns. This is particularly important for the manufacturing of precision nozzles used in helium-filled soap bubble generators.

The use of additive manufacturing to construct enclosures is of particular interest throughout this project due to the ability to manufacturer encolusres with most, if not all, mounts and holes pre-fabricated. This low barrier to fabrication is of particular importance since many potential users of OpenSync synchronizers would be undergraduate students who may lack expertise in manufacturing custom equipment. Moreso, it would be particularly cumbersome to machine 15 holes for SMB jacks and a further 3 for status LEDs, especially for metallic enclosures. To help minimize possible difficulties during hte manufacturing processes, all designs should careffully bear in mind the current liomitations of filamet and resin-based additive manufacturing technologies.

## 2.2 Input/Output Channels
OpenSync is a 16 channel synchronizer that includes a further 3 input channels to help fascilitate advanced fluid flow measurements. The input and output channels are arranged in three rows of five with a gap of 1 cm between the centers of each jack for the input and output channels respectively. Due to the tight proximity of which the input and output jacks are placed, the jacks utilized on this device would be of the type SMB instead of the more common BNC connectors/jacks. SMB connectors do not require a twisting action to create an attachment and instead operate on the basis of friction. This may help reduce the amount of fideling required to connect equipment to the synchronizer. All input and output ports use 50 Ohm resistance at 5V since most laboratory equipment use this level of impedance. 

## 2.3 Microcontroller Circuitry
The main selling point of OpenSync is its low cost of aquisition. This is particularly attributed to the electronics used on the PCB which are based around microcontroller technology. The main sequence processor is the RP2350B (QFN-80 package) microcontroller which runs on 3.3V with rather basic fault tolerance for 5V overvoltages. To minimize the risk of damage to the general purpose input/output (GPIO) pins, a logic level translator is used to profive a buffer between 5V outputs and the 3.3V operatuing voltage of the microcontroller. It is of particular importance that the internal circuit (IC) used for translating 3.3V logic to 5V logic allows for high throughput and low rise/fall times. For this particular case, the SN74LVC8T245 transciever provides a reliable form of logic translation with rise and fall times on the order of nanoseconds on the single digit scale. To provide 5V tolerance to the input channels of the synchronizer, Schmidt triggers are a simple and fast IC that can natively provide 5V tolerance for 3.3V microcontroller oeprating voltages. Beyond ICs that provide 5V tolerances, all other ICs such as the crystal oscilator, 5V to 3V power conversion, and more are nearly identical to that of the RP2350 hardware minimal design example provided by the Raspberry Pi Foudnation.