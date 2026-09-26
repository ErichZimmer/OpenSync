.. _api_scpi_pulse_reference:

================================
``PULSe`` Submodule Reference
================================

Pulse sequencer properties of an OpenSync device can be accessed using the
``PULSe`` root path. Pulse sequencer configuration can be set when the device
is idle or aborted. Queries can also be used during device operation.

Commands that include ``PULSe<N>`` operate on sequencer ``<N>``. Sequencer 0 is
the T0 clock, and sequencers 1 through 8 are output channels CHA through CHH.
Commands that omit ``<N>`` operate on T0. For example, ``:PULSe1:STATe ON``
enables output channel CHA, while ``:PULSe:STATe ON`` enables T0.


.. _scpi_pulse_state:

``:STATe``
===========

 | :PULSe<N>:STATe?
 | :PULSe<N>:STATe ON | OFF

This command enables or disables sequencer ``<N>``. Sequencers that are not
enabled will not be processed during program execution. The command accepts
`ON` or `1` to enable and `OFF` or `0` to disable. The query returns `1` or `0`.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:STAT ON
   :PULS1:STAT?
   >>> 1

.. note::
 * \*RST resets ``:PULSe<N>:STATe`` to `OFF`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_divider:

``:DIVider``
============

 | :PULSe0:DIVider?
 | :PULSe<N>:DIVider <positive integer>

This command sets the clock divider of sequencer ``<N>``. The divider changes
the clock resolution and scales the minimum and maximum period or delay that
can be configured.

.. csv-table:: Divider Data Description
   :header: "Clock Divider", "Unit Time Scale"
   :widths: 15, 25

   "1", "Clock resolution is 4 ns"
   "2", "Clock resolution is 8 ns"
   "5", "Clock resolution is 20 ns"
   "25", "Clock resolution is 100 ns"
   "250", "Clock resolution is 1,000 ns (1 us)"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:DIV 2
   :PULS0:DIV?
   >>> 2

.. note::
 * \*RST resets ``:PULSe<N>:DIVider`` to `1`.
 * Configuration commands are not allowed during device operation.
 * Program configuration supports dividers up to `65500` for T0 and output channels.


.. _scpi_pulse_period:

``:PERiod``
============

 | :PULSe0:PERiod?
 | :PULSe0:PERiod <time>

This command sets the clock period used by T0. This command is available only
for sequencer 0. The allowed period is from 320 ns multiplied by the clock
divider through 10 s multiplied by the clock divider.

Time values without a suffix are in seconds. Supported time suffixes are
`NS`, `US`, `MS`, `S`, and `MIN`. The query returns the period in
seconds.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:PER 1MS
   :PULS0:PER?
   >>> 0.001

.. note::
 * \*RST resets ``:PULSe0:PERiod`` to `0`. Set a valid period before starting T0.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_bcounter:

``:BCOunter``
==============

 | :PULSe<N>:BCOunter?
 | :PULSe<N>:BCOunter <count>

This command sets the B counter of sequencer ``<N>``. Values range from `0`
through `30,000,000`. For T0, the B counter sets the number of acquisitions, each
containing the configured P and O counts. For an output channel, the B counter
sets the total number of waveform buffers to execute.

A value of `0` selects infinite operation until T0 is stopped or the program is
aborted using ``:DEVice:STOP``. The query returns the configured count.

.. note::
   The `30,000,000` maximum keeps finite B counts within the RP2350 DMA
   transfer-count limit for both T0 and output channels.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:BCO 5
   :PULS1:BCO?
   >>> 5

.. note::
 * \*RST resets ``:PULSe<N>:BCOunter`` to `0`.
 * B counts are not decremented for pulse inhibit, but are decremented on output inhibit gate modes.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_pcounter:

``:PCOunter``
==============

 | :PULSe<N>:PCOunter?
 | :PULSe<N>:PCOunter <count>

This command sets the ON duty cycle counter of sequencer ``<N>``. Values range
from `0` through `1,000,000,000`. T0 produces an event for each ON period. An output
channel executes its waveform buffer for each accepted ON synchronization
event. After the P count, the sequencer skips the number of events specified
by the O counter and repeats the P/O cycle.

Output channels treat a P count of `0` as `1` when configuring the sequencer.
Use a P count of at least `1` for T0. The query returns the configured count.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:PCO 3
   :PULS1:PCO?
   >>> 3

.. note::
 * \*RST resets ``:PULSe<N>:PCOunter`` to `1`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_ocounter:

``:OCOunter``
==============

 | :PULSe<N>:OCOunter?
 | :PULSe<N>:OCOunter <count>

This command sets the OFF duty cycle counter of sequencer ``<N>``. Values range
from `0` through `1,000,000,000`. After the P count, T0 skips this number of clock
periods, or an output channel skips this number of synchronization events,
before starting the next P/O cycle. A value of `0` means no OFF events are
skipped. The query returns the configured count.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:OCO 2
   :PULS1:OCO?
   >>> 2

.. note::
 * \*RST resets ``:PULSe<N>:OCOunter`` to `0`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_wcounter:

``:WCOunter``
==============

 | :PULSe<N>:WCOunter?
 | :PULSe<N>:WCOunter <count>

This command sets the initial wait counter of output channel ``<N>``, where
``<N>`` is 1 through 8. Use values from `0` through `1,000,000,000`. The channel skips
this number of events from its selected synchronization source before starting
its P/O cycle. A value of `0` starts on the first accepted event. The initial
wait is performed once per program run. The query returns the configured count.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:WCO 2
   :PULS1:WCO?
   >>> 2

.. note::
 * \*RST resets ``:PULSe<N>:WCOunter`` to `0`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_sync:

``:SYNC``
==========

 | :PULSe<N>:SYNC?
 | :PULSe<N>:SYNC T0 | CHA | CHB | CHC | CHD | CHE | CHF | CHG | CHH

This command sets the synchronization source of output channel ``<N>``, where
``<N>`` is 1 through 8. The channel waits for rising events from the selected
source. `T0` selects the T0 clock. `CHA` through `CHH` select output channels
1 through 8, respectively. A channel cannot select itself as its source.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:SYNC T0
   :PULS1:SYNC?
   >>> T0

.. note::
 * \*RST resets ``:PULSe<N>:SYNC`` to `T0`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_buffer:

``:BUFfer``
============

 | :PULSe<N>:BUFfer?
 | :PULSe<N>:BUFfer <state>,<delay>[,<state>,<delay>...]

This command sets the waveform buffer of output channel ``<N>``, where ``<N>``
is 1 through 8. The buffer contains one through six output-state and delay
pairs. Each state is `ON` or `1` for HIGH, or `OFF` or `0` for LOW. The following
delay specifies how long that state is held.

Time values without a suffix are in seconds. Supported time suffixes are
`PS`, `NS`, `US`, `MS`, `S`, `MIN`, and `HR`. Each supplied delay must be from
44 ns multiplied by the channel clock divider through 8 s multiplied by the
channel clock divider.

Each command replaces the channel buffer. Unused pairs are cleared to `OFF`
and `0`. The query returns all six pairs, with delays in seconds.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:BUF ON,250MS,OFF,500MS,ON,250MS,OFF,500MS,ON,250MS,OFF,500MS
   :PULS1:BUF?
   >>> ON,0.25,OFF,0.5,ON,0.25,OFF,0.5,ON,0.25,OFF,0.5

.. note::
 * \*RST resets ``:PULSe<N>:BUFfer`` to six `OFF,0` pairs.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_reset:

``:RESet``
==========

 | :PULSe<N>:RESet

This command resets sequencer ``<N>`` to its default configuration. The channel
is disabled, the divider and P counter are set to `1`, and the remaining
settings and buffers are cleared to their defaults.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:RES

.. note::
 - \*RST resets all sequencers.
 - Command is not allowed during device operation.


=============================
``:OUTPut`` Properties
=============================
``OUTPut`` is a subdirectory that controls the output logic level of channels
1 through 8.


.. _scpi_pulse_output_level:

``:LEVel``
===========

 | :PULSe<N>:OUTPut:LEVel?
 | :PULSe<N>:OUTPut:LEVel LVTtl | TTL

This command selects the output logic level of output channel ``<N>``, where
``<N>`` is 1 through 8.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:OUTP:LEV TTL
   :PULS1:OUTP:LEV?
   >>> TTL

.. note::
 * \*RST resets ``:PULSe<N>:OUTPut:LEVel`` to `LVTtl`.
 * Configuration commands are not allowed during device operation.


=============================
``:GATe`` Properties
=============================
``GATe`` is a subdirectory that controls global gating through T0. These
commands are available only for sequencer 0.


.. _scpi_pulse_gate_mode:

``:MODe``
==========

 | :PULSe0:GATe:MODe?
 | :PULSe0:GATe:MODe DISabled | PULSe | OUTPut | CHANnel

This command selects how the external gate affects program execution. An
asserted gate inhibits the activity selected by the gate mode. The active gate
level is set by ``:PULSe0:GATe:LOGic``.

.. csv-table:: Gate Mode Description
   :header: "SCPI String", "Description"
   :widths: 15, 45

   "``DISabled``", "Global gating is disabled"
   "``PULSe``", "An asserted gate prevents a new acquisition from starting; an acquisition already started continues"
   "``OUTPut``", "An asserted gate suppresses T0 output events while the P/O counters continue"
   "``CHANnel``", "Gating is controlled by each output channel's CGATe settings"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:GAT:MOD CHANNEL
   :PULS0:GAT:MOD?
   >>> CHANnel

.. note::
 * \*RST resets ``:PULSe0:GATe:MODe`` to `DISabled`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_gate_logic:

``:LOGic``
===========

 | :PULSe0:GATe:LOGic?
 | :PULSe0:GATe:LOGic LOW | HIGH

This command selects the active level of the global gate input. `LOW` asserts
the gate when the input is LOW, and `HIGH` asserts the gate when the input is
HIGH.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:GAT:LOG HIGH
   :PULS0:GAT:LOG?
   >>> HIGH

.. note::
 * \*RST resets ``:PULSe0:GATe:LOGic`` to `LOW`.
 * Configuration commands are not allowed during device operation.


=============================
``:CGATe`` Properties
=============================
``CGATe`` is a subdirectory that controls gating for individual output channels.
These settings are used when ``:PULSe0:GATe:MODe`` is `CHANnel`.


.. _scpi_pulse_cgate_mode:

``:MODe``
==========

 | :PULSe<N>:CGATe:MODe?
 | :PULSe<N>:CGATe:MODe DISabled | PULSe | OUTPut

This command selects the gate mode of output channel ``<N>``, where ``<N>`` is
1 through 8. Set ``:PULSe0:GATe:MODe CHANnel`` before setting a channel gate
mode. The active gate level is set by ``:PULSe<N>:CGATe:LOGic``.

.. csv-table:: Channel Gate Mode Description
   :header: "SCPI String", "Description"
   :widths: 15, 45

   "``DISabled``", "Channel gating is disabled"
   "``PULSe``", "An asserted gate rejects synchronization events without advancing the P/O counters; an accepted waveform continues"
   "``OUTPut``", "An asserted gate writes LOW at waveform segment updates while waveform timing and the P/O counters continue"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:GAT:MOD CHANNEL
   :PULS1:CGAT:MOD PULSE
   :PULS1:CGAT:MOD?
   >>> PULSe

.. note::
 * \*RST resets ``:PULSe<N>:CGATe:MODe`` to `DISabled`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_cgate_logic:

``:LOGic``
===========

 | :PULSe<N>:CGATe:LOGic?
 | :PULSe<N>:CGATe:LOGic LOW | HIGH

This command selects the active gate level of output channel ``<N>``, where
``<N>`` is 1 through 8. `LOW` asserts the gate when the input is LOW, and `HIGH`
asserts the gate when the input is HIGH.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS1:CGAT:LOG HIGH
   :PULS1:CGAT:LOG?
   >>> HIGH

.. note::
 * \*RST resets ``:PULSe<N>:CGATe:LOGic`` to `LOW`.
 * Configuration commands are not allowed during device operation.


=============================
``:TRIGger`` Properties
=============================
``TRIGger`` is a subdirectory that controls external triggering of T0. These
commands are available only for sequencer 0.


.. _scpi_pulse_trigger_mode:

``:MODe``
==========

 | :PULSe0:TRIGger:MODe?
 | :PULSe0:TRIGger:MODe DISabled | TRIGgered

This command enables or disables external triggering. `TRIGgered` starts one
T0 acquisition on the selected external trigger edge. Each acquisition contains
the configured P and O counts. `DISabled` disables external triggering.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:TRIG:MOD TRIGGERED
   :PULS0:TRIG:MOD?
   >>> TRIGgered

.. note::
 * \*RST resets ``:PULSe0:TRIGger:MODe`` to `DISabled`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_trigger_edge:

``:EDGe``
==========

 | :PULSe0:TRIGger:EDGe?
 | :PULSe0:TRIGger:EDGe RISing | FALLing

This command selects the external trigger edge used when
``:PULSe0:TRIGger:MODe`` is `TRIGgered`.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:TRIG:EDG RISING
   :PULS0:TRIG:EDG?
   >>> RISing

.. note::
 * \*RST resets ``:PULSe0:TRIGger:EDGe`` to `RISing`.
 * Configuration commands are not allowed during device operation.
