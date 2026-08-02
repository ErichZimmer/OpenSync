# Design and Optimizations for the OpenSync Firmware

## Version History
| Date | Version | Change | 
| --- | --- | --- |
| 08/01/2026 | 0.1.0 | Intitial publishment. |

## Key Words
- DMA - Direct Memory Access
- FIFO - First in, First out
- FPGA - Field Programmable Gate Array
- ISR - Input Shift Register
- OSR - Output Shift Register
- PIO - Programmable Input/Output
- GPIO - General Purpose Input/Output
- Word - A 32 bit variable in assembly language

## 1. Introduction
As standing, the OpenSync synchronizer provides a low cost means for controlling laboratory equipment. Utilizing an arbitrary pulse generator as the core pulse processing firmware, a single assembly word can controll all eight (8) channels with the ability to control up to 32 channels per word (each word is 32 bits for the current microcontroller used in this project). For a basic PIV experiment, the arbitrary pulse generator design appears to be wholefully sufficient. However, a shift towards more standardized commands for digital pulse generators is currently underway. This shift was originally inspired after using a [Quantum Composers 9420](https://quantumcomposers.myshopify.com/products/9420-series-delay-pulse-generator?variant=45322235478335) digital delay generator which implements each channel as independent pulse generators synchonized to a time base. This means that each output channel is completely independent of other output channels unlike arbitrary pulse generators where a single output word controls all output states. While implementing similar functionality to the Quantom Composers digital delay generator would certainly make the OpenSync firmware more complicated, the expected return is that an OpenSync device would share nearly all capabilities of its commercial counterparts. This design choice is expected to allow an OpenSync device to be used in a greater variety of experimental conditions.

## 2. Updated Design Philosophy

### 2.1 SCPI Interface Changes
To begin, the SCPI interface would have a significant overhall. A separation of concerns issue has been identified where the SCPI parameters share variables in a struct that is directly used in the programmable input/output (PIO) state machines. This convolution of mixing operational boundaries has resulted in an interesting conglomerate of firmware code that relies on setter and getters along with direct access to static structs through a dedicated setter that return their pointers. As such, separating the internal mechanism and the SCPI interface will be the first task to be operated on. The separation of internal data and SCPI data can be accomplished through two sets of c structs: one for state machine configuration and one for SCPI interface configuration. By having dedicated structs for SCPI interfaces, the modified, stored, and retrieved parameters can remain in human-readable form instead of the previous numerical way. Once the SCPI structs have been configured, they are converted into the internal structs used directly in the state machines via conversion functions. By designing the interface structs in this way, the SCPI interface and internal mechanics would be properly separated.

### 2.2 Clock Sequencing Changes
The next task to accomplish the configuration of the clock source. Currently, the clock source resolution is configured through the clock sequencer to enable independent clock dividers for each state machine. While this has the potential to enable relatively advance timing schemes that are bound by timing restrictions, it has been deemed that this feature is unecessary. Instead, the clock source would be set as a system timing resolution for all utilized state mahcines. Beyond system resolution, the current three clock sequencers would be revised into a single clock sequencer dubbed `T0`. `T0` is the master timing event which is used to synchronize all output channels. This master timer is similar to that used in the Quantum Composer digital delay generators, however with differences in approach due to the way OpenSync clock sequencer currently function. The gist of how `T0` operates is given below. 

| Parameter | Values | Description | 
| --- | --- | --- |
| Mode | Continuous, Single Shot, Burst Mode, Duty Cycle | Determines the pattern of `T0` events generated after the sequencer is started. | 
| COntrol Source | Immediate, External, Internal | Determines the event or command that starts or controls `T0` event generation. |
| External Control Mode | Start, Event, Gate | Determines how an external input affects `T0`.  It may start an internally timed sequence, produce one T0 per accepted edge, or permit/inhibit internally timed T0 events. |
| Trigger Edge | Rising, Falling | Determines the slope which activates an external trigger event. |
| Gate Level | Low, High | Determines the level which inhibits events. |
| Period Units | US, MS, S | Sets the units for the frequency/period parameters |
| Period | Double | Defines the time between internally generated `T0` events. |
| Burst Count | Integer | Defines the number of `T0` events to generate during one event. |
| Duty On Count | Integer | Defines the numver of `T0` events to generate during the active portion of a duty cycle. |
| Duty Off Count | Integer| Defines the number of `T0` periods supressed during the inactice portion of a duty cycle. |
| Delay Units | NS, US, MS, S | Sets the units for the delay parameters. |
| Trigger Skip | Integer | Defines the number of external edges to ignore before an edge is accepted. |
| Trigger Delay | Double | A double that adds a dellay between an accepted trigger edge and the `T0` event signal. |
| Count | Integer | The number of `T0` events to perform before disarming. |

It should be noted that `T0` is not the system clock timer. It represents the event signal that is sent to the output channels. Additionally, there are a few deviations from the conventions put forth by Quantom Composers. For instance, period is measured in units instead of clock ticks. This means that setting units to `US` (microseconds) and a period of 100.0 would generate a frequency of ten kilohertz. The same can be said for external trigger delays which are also measured in the selected units. This is a convienience feature that is highly likely to remain in the new OpenSync firmware due to the designer liking that feature. Finally, the variable frequency portion of the clock sequencer will be removed for the time being.


### 2.3 Output Sequencing Changes
The biggest shift in this firmware revision is the move from implementing an arbitrary pulse generator to eight independent pulse generators. The independent pulse generators inherit meny of the parameters of the master timer in addition to including a event source and pulse data. The existing 16 word memory aligned instruction buffer would be replicated for each channel and modify the state of a single gpio pin as opppose to a contiguous group of gpio pins. This would require eight state machines to fullfill an eight channel synchronizer and would likely increase channel-to-channel jitter due to all the state machines being utilized. However, the increase in jitter is expected to be marginal (yet to be confirmed with an osciliscope). Output channels are identified using letters a though h (e.g., CHA to CHH) which map to state machines 0 to 3 on PIO blocks 0 and 1. A list of output configuration parameters are given below.

| Parameter | Values | Description |
| --- | --- | --- |
| Channel | A–H | Selects the output channel being configured. |
| Enabled | Enabled, Disabled | Determines whether the channel accepts synchronization events and executes its instruction buffer. |
| Synchronization Source | `T0`, Channel A–H | Selects the event source used to trigger execution of the output sequence. |
| Source Units | NS, US, MS, S | Sets the units used for the source delay values. |
| Instruction Buffer | 16 × 32-bit Words | Raw instruction buffer containing two header words, six output-state/output-delay pairs, and two terminating words. |
| Source Skip | Integer | Header word 0. Defines the number of `T0` or selected-source events ignored before the next event is accepted. |
| Source Delay | Double | Header word 1. Defines the delay between the accepted source event and the first output-state/output-delay pair. |
| Output Units | NS, US, MS, S | Sets the units used for the output delay values. |
| Output State | Low, High | Defines the channel state applied during the corresponding output-delay interval. |
| Output Delay | Double | Defines how long the corresponding output state is maintained. |
| Output-State/Output-Delay Pairs | Six Pairs | Words 2–13 contain six ordered output-state/output-delay pairs. |
| Terminating Words | Two 32-bit Words | Words 14 and 15 terminate the instruction sequence. The channel is forced low after termination. |
