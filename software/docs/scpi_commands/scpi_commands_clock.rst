.. _api_scpi_clock_reference:

========================================
``SOURce:CLOCk`` Submodule Reference
========================================

Clock sequencer properties of an OpenSync device can be accessed using the
``SOURce:CLOCk`` root path. Clock sequencer configuration and information can be
set and queried when the device is idle and not currently executing a program.

Commands that include ``CLOCk#`` may be addressed with a numeric suffix, such as
``CLOCk0``, ``CLOCk1``, or ``CLOCk2``. If no suffix is supplied, the currently
selected clock sequencer index is used.


.. _scpi_clock_select:

``:SELect``
===========


 | :SOURce:CLOCk:SELect?
 | :SOURce:CLOCk:SELect 0 | 1 | 2

This command selects the clock sequencer index for stateful configuration
operations. The sequencer IDs reflect the clock sequencer index where 0 is clock
sequencer 0 and so on.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC:SEL 0
   :SOUR:CLOC:SEL?
   >>> 0

.. note::
 * \*RST resets ``:SOURce:CLOCk:SELect`` to `0`.
 * The query command returns the currently selected stateful clock sequencer index.


.. _scpi_clock_state:

``:STATe``
==========


 | :SOURce:CLOCk<N>:STATe?
 | :SOURce:CLOCk<N>:STATe ON | OFF

This command enables or disables the clock sequencer at sequencer <N> if stated,
or the selected sequencer if not. Clock sequencers that are not enabled will not
be processed during program execution.

.. csv-table:: State Parameters
   :header: "SCPI String", "Description"
   :widths: 15, 25

   "``ON``", "Enable the selected clock sequencer"
   "``OFF``", "Disable the selected clock sequencer"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:STAT ON
   :SOUR:CLOC0:STAT?
   >>> 1

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:STATe`` to `OFF`.
 * Command is not allowed during device operation.


.. _scpi_clock_divider:

``:DIVider``
============

 | :SOURce:CLOCk<N>:DIVider?
 | :SOURce:CLOCk<N>:DIVider HIGH | HIGH_RES | MED | MED_RES | LOW | LOW_RES | VERY_LOW | VERY_LOW_RES | VERY_VERY_LOW | VERY_VERY_LOW_RES

This command sets the time scale of the clock sequencer at sequencer <N> if
stated, or the selected sequencer if not. The string arguments are converted to a
clock divider that changes the clock resolution of the clock sequencer. For
example, ``HIGH_RES`` results in a clock divider of `1` while ``LOW_RES`` results
in a clock divider of `5`.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Clock Divider", "Unit Time Scale"
   :widths: 18, 8, 18

   "``HIGH_RES`` or ``HIGH``", "1", "Clock resolution is 4 ns"
   "``MED_RES`` or ``MED``", "2", "Clock resolution is 8 ns"
   "``LOW_RES`` or ``LOW``", "5", "Clock resolution is 20 ns"
   "``VERY_LOW_RES`` or ``VERY_LOW``", "25", "Clock resolution is 100 ns"
   "``VERY_VERY_LOW_RES`` or ``VERY_VERY_LOW``", "250", "Clock resolution is 1,000 ns (1 us)"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:DIV MED_RES
   :SOUR:CLOC0:DIV?
   >>> 2

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:DIVider`` to `HIGH_RES`.
 * Command is not allowed during device operation.
 * Cached clock sequences need to be re-applied when the clock divider is changed.
 * Trigger delay settings are converted through the selected clock divider and should be re-applied when the divider is changed.


.. _scpi_clock_mode:

``:MODe``
=========

 | :SOURce:CLOCk<N>:MODe?
 | :SOURce:CLOCk<N>:MODe INTernal | EXTernal

This command sets the clock sequencer operating mode at sequencer <N> if stated,
or the selected sequencer if not. The clock mode determines whether the clock
sequencer runs from its internal/free-run behavior or from the external clock
sequencer mode used by the firmware.

.. csv-table:: Clock Mode Parameters
   :header: "SCPI String", "Description"
   :widths: 15, 30

   "``INTernal``", "Use the internal/free-run clock sequencer mode"
   "``EXTernal``", "Use the external/sequencer clock mode"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:MODE INT
   :SOUR:CLOC0:MODE?
   >>> 0

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:MODe`` to the firmware default clock mode.
 * Command is not allowed during device operation.
 * Query returns the internal numeric mode value stored by the firmware.


.. _scpi_clock_units:

``:UNITs``
==========

 | :SOURce:CLOCk<N>:UNITs?
 | :SOURce:CLOCk<N>:UNITs HZ | KHZ | MHZ

This command sets the frequency units of the clock sequencer at sequencer <N> if
stated, or the selected sequencer if not. The string arguments are converted to a
unit scaler used when cached frequency data are applied to the clock instruction
buffer. For example, ``HZ`` results in a unit scaler of `1.0`, ``KHZ`` results in
`1.0e3`, and ``MHZ`` results in `1.0e6`.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Unit Scale", "Description"
   :widths: 12, 12, 25

   "``HZ``", "1.0", "Clock frequency buffer data are in hertz"
   "``KHZ``", "1.0e3", "Clock frequency buffer data are in kilohertz"
   "``MHZ``", "1.0e6", "Clock frequency buffer data are in megahertz"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:UNIT KHZ
   :SOUR:CLOC0:UNIT?
   >>> 1000.0

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:UNITs`` to `HZ`.
 * Command is not allowed during device operation.
 * Cached clock frequency data need to be re-applied when data units are changed.


.. _scpi_clock_data:

``:DATA``
=========


 | :SOURce:CLOCk<N>:DATA?

This command queries the instruction buffer of the clock sequencer at sequencer
<N> if stated, or the selected sequencer if not. This is read only and cannot be
set directly. Frequency and count data must first be written to the static data
buffer, then applied using ``:SOURce:CLOCk<N>:DATA:BUFFer:APPly``.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:DATA?
   >>> 0,0,0,0,0,0...

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:DATA`` to all zeros.
 * Query returns the internal clock instruction buffer as unsigned integer values.
 * Command is not allowed during device operation.


.. _scpi_clock_reset:

``:RESet``
==========

 | :SOURce:CLOCk<N>:RESet

This command resets the clock sequencer at sequencer <N> if stated, or the
selected sequencer if not.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:RES

.. note::
 * \*RST calls this command on all clock sequencers.
 * Command is not allowed during device operation.


===================================
``:DATA:BUFFer`` Properties
===================================
``DATA:BUFFer`` is a subdirectory that controls the static instruction buffers
within the clock sequencer SCPI interface. These buffers are intermediary and are
not applied to a clock sequencer until the ``:APPly`` command is executed.

The frequency and count buffers are global/static SCPI-side buffers. The clock
sequencer ID is still parsed and validated by these commands, but the cached
buffer contents are not assigned to a specific clock sequencer until
``:SOURce:CLOCk<N>:DATA:BUFFer:APPly`` is executed.


.. _scpi_clock_data_buffer_frequency:

``:FREQuency``
==============

 | :SOURce:CLOCk<N>:DATA:BUFFer:FREQuency?
 | :SOURce:CLOCk<N>:DATA:BUFFer:FREQuency <list of doubles>

This command caches clock frequency instructions into a static internal buffer.
Frequency values are interpreted using the currently selected
``:SOURce:CLOCk<N>:UNITs`` setting when the buffer is applied.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:DATA:BUFF:FREQ 7.662,15
   :SOUR:CLOC0:DATA:BUFF:FREQ?
   >>> 7.662,15.0,0,0,0...

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:DATA:BUFFer:FREQuency`` to all zeros.
 * Command is not allowed during device operation.
 * Cached frequency values need to be applied before they are used by a clock sequencer.
 * Frequency values are converted to periods, then converted to clock cycles during ``:APPly``.


.. _scpi_clock_data_buffer_count:

``:COUNt``
==========

 | :SOURce:CLOCk<N>:DATA:BUFFer:COUNt?
 | :SOURce:CLOCk<N>:DATA:BUFFer:COUNt <list of uint32_t>

This command caches clock repetition/count instructions into a static internal
buffer. Each count value pairs with the frequency value at the same list index.
The frequency and count buffers must have the same number of supplied elements
before the buffer can be applied.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:DATA:BUFF:COUN 5,10
   :SOUR:CLOC0:DATA:BUFF:COUN?
   >>> 5,10,0,0,0...

.. note::
 * \*RST resets ``:SOURce:CLOCk<N>:DATA:BUFFer:COUNt`` to all zeros.
 * Command is not allowed during device operation.
 * Cached count values need to be applied before they are used by a clock sequencer.
 * Applying buffers with different supplied frequency and count list lengths raises a list-length error.


.. _scpi_clock_data_buffer_clear:

``:CLEar``
==========

 | :SOURce:CLOCk<N>:DATA:BUFFer:CLEar

This command clears the cached frequency and count buffers and sets them all to
zero.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:DATA:BUFF:CLE
   :SOUR:CLOC0:DATA:BUFF:FREQ?
   >>> 0,0,0,0,0...

.. note::
 * \*RST calls this command.
 * Command is not allowed during device operation.


.. _scpi_clock_data_buffer_apply:

``:APPly``
==========

 | :SOURce:CLOCk<N>:DATA:BUFFer:APPly

This command loads the currently cached frequency and count buffers into clock
sequencer <N> if stated, or the selected sequencer if not. Frequency values are
converted to periods in nanoseconds and then to clock cycles using the currently
selected frequency units and clock divider. Values that are invalid during the
conversion process will raise a `data out of range` error.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:CLOC0:DATA:BUFF:FREQ 10,20
   :SOUR:CLOC0:DATA:BUFF:COUN 5,10
   :SOUR:CLOC0:DATA:BUFF:APP

.. note::
 * Command is not allowed during device operation.
 * Cached parameters are converted using the current data units and clock divider.
 * Cached frequency and count buffers must have the same number of supplied elements.
 * Cached clock sequences need to be re-applied when data units or the clock divider are changed.


====================================
``TRIGger:CLOCk`` Submodule Reference
====================================
``TRIGger:CLOCk`` controls the trigger behavior for each clock sequencer. These
commands replace the old ``:CLOCk<N>:TRIGger:*`` command layout. Trigger settings
are now addressed using the top-level ``TRIGger`` root path followed by the clock
sequencer suffix.

Commands that include ``CLOCk#`` may be addressed with a numeric suffix, such as
``CLOCk0``, ``CLOCk1``, or ``CLOCk2``. If no suffix is supplied, the currently
selected clock sequencer index is used.


.. _scpi_clock_trigger_mode:

``:MODe``
=========

 | :TRIGger:CLOCk<N>:MODe?
 | :TRIGger:CLOCk<N>:MODe IMMediate | EDGE | GATE

This command sets the trigger source mode of clock sequencer <N> if stated, or
the selected sequencer if not. Trigger mode selection is now separate from the
trigger edge and gate level settings.

.. csv-table:: Trigger Mode Parameters
   :header: "SCPI String", "Description"
   :widths: 15, 35

   "``IMMediate``", "Use immediate/internal triggering"
   "``EDGE``", "Trigger from an external edge selected by ``:EDGE``"
   "``GATE``", "Use external gate behavior selected by ``:GATE:LEVel``"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:MODE EDGE
   :TRIG:CLOC0:MODE?
   >>> 1

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:MODe`` to the firmware default trigger mode.
 * Command is not allowed during device operation.
 * Query returns the internal numeric trigger source value stored by the firmware.


.. _scpi_clock_trigger_edge:

``:EDGE``
=========

 | :TRIGger:CLOCk<N>:EDGE?
 | :TRIGger:CLOCk<N>:EDGE POSitive | NEGative

This command sets the external trigger edge for clock sequencer <N> if stated,
or the selected sequencer if not. This setting is used when
``:TRIGger:CLOCk<N>:MODe`` is set to ``EDGE``.

.. csv-table:: Trigger Edge Parameters
   :header: "SCPI String", "Description"
   :widths: 15, 30

   "``POSitive``", "Trigger on a positive/rising edge"
   "``NEGative``", "Trigger on a negative/falling edge"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:EDGE POS
   :TRIG:CLOC0:EDGE?
   >>> 0

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:EDGE`` to the firmware default trigger edge.
 * Command is not allowed during device operation.
 * Query returns the internal numeric edge value stored by the firmware.


.. _scpi_clock_trigger_gate_level:

``:GATE:LEVel``
===============

 | :TRIGger:CLOCk<N>:GATE:LEVel?
 | :TRIGger:CLOCk<N>:GATE:LEVel HIGH | LOW

This command sets the gate-active level for clock sequencer <N> if stated, or
the selected sequencer if not. This setting is used when
``:TRIGger:CLOCk<N>:MODe`` is set to ``GATE``.

.. csv-table:: Gate Level Parameters
   :header: "SCPI String", "Description"
   :widths: 15, 30

   "``HIGH``", "Gate is active when the external gate signal is high"
   "``LOW``", "Gate is active when the external gate signal is low"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:GATE:LEV HIGH
   :TRIG:CLOC0:GATE:LEV?
   >>> 0

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:GATE:LEVel`` to the firmware default gate level.
 * Command is not allowed during device operation.
 * Query returns the internal numeric gate level value stored by the firmware.


.. _scpi_clock_trigger_units:

``:UNITs``
==========

 | :TRIGger:CLOCk<N>:UNITs?
 | :TRIGger:CLOCk<N>:UNITs NS | US | MS | S | M | H

This command sets the units of the clock sequencer trigger-delay system at
sequencer <N> if stated, or the selected sequencer if not. The string arguments
are converted to a unit scaler that converts the supplied trigger delay to
nanoseconds before it is converted to clock cycles.

.. csv-table:: Trigger Unit Parameters
   :header: "SCPI String", "Unit Scale", "Description"
   :widths: 10, 12, 30

   "``NS``", "1", "Trigger delay data are in nanoseconds"
   "``US``", "1.0e3", "Trigger delay data are in microseconds"
   "``MS``", "1.0e6", "Trigger delay data are in milliseconds"
   "``S``", "1.0e9", "Trigger delay data are in seconds"
   "``M``", "60.0e9", "Trigger delay data are in minutes"
   "``H``", "3600.0e9", "Trigger delay data are in hours"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:UNIT US
   :TRIG:CLOC0:UNIT?
   >>> 1000.0

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:UNITs`` to the firmware default trigger delay unit.
 * Command is not allowed during device operation.
 * Trigger delay data need to be re-applied when trigger units are changed.


.. _scpi_clock_trigger_input:

``:INPut``
==========


 | :TRIGger:CLOCk<N>:INPut?
 | :TRIGger:CLOCk<N>:INPut 0 | 1 | 2

This command sets the trigger input pin of clock sequencer <N> if stated, or the
selected sequencer if not. The pin IDs reflect the trigger input index where 0 is
trigger input 0 and so on.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:INP 0
   :TRIG:CLOC0:INP?
   >>> 0

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:INPut`` to the firmware default trigger input.
 * Command is not allowed during device operation.


.. _scpi_clock_trigger_delay:

``:DELay``
==========

 | :TRIGger:CLOCk<N>:DELay?
 | :TRIGger:CLOCk<N>:DELay <double>

This command sets the trigger delay for clock sequencer <N> if stated, or the
selected sequencer if not. The supplied delay is interpreted using the currently
selected ``:TRIGger:CLOCk<N>:UNITs`` setting. It is then converted to nanoseconds
and finally converted to clock cycles using the selected clock divider.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:UNIT US
   :TRIG:CLOC0:DEL 5.0
   :TRIG:CLOC0:DEL?
   >>> 1250

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:DELay`` to the firmware default trigger delay.
 * Command is not allowed during device operation.
 * Query returns the stored trigger delay in clock cycles, not the original user-supplied time value.
 * Trigger delay should be re-applied when trigger units or the clock divider are changed.


.. _scpi_clock_trigger_skip:

``:SKIP``
=========

 | :TRIGger:CLOCk<N>:SKIP?
 | :TRIGger:CLOCk<N>:SKIP <uint32_t>

This command sets the number of external trigger events to skip before a trigger
is accepted by clock sequencer <N> if stated, or the selected sequencer if not.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:SKIP 7
   :TRIG:CLOC0:SKIP?
   >>> 7

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:SKIP`` to the firmware default trigger skip count.
 * Command is not allowed during device operation.


.. _scpi_clock_trigger_count:

``:COUNt``
==========

 | :TRIGger:CLOCk<N>:COUNt?
 | :TRIGger:CLOCk<N>:COUNt <uint32_t>

This command sets the trigger repetition count for clock sequencer <N> if stated,
or the selected sequencer if not. This value controls how many times the trigger
configuration is repeated by the clock sequencer firmware.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :TRIG:CLOC0:COUN 10
   :TRIG:CLOC0:COUN?
   >>> 10

.. note::
 * \*RST resets ``:TRIGger:CLOCk<N>:COUNt`` to the firmware default trigger count.
 * Command is not allowed during device operation.
