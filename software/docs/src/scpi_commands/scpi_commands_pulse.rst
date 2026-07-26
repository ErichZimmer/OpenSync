.. _api_scpi_pulse_reference:

================================
``PULSe`` Submodule Reference
================================

Pulse sequencer properties of an OpenSync device can be accessed using the
``SOURce:PULSe`` root path. Pulse sequencer configuration and information can be
set and queried when the device is idle and not currently executing a program.

Commands that include ``PULSe<N>`` operate on pulse sequencer ``<N>``. Commands
that omit ``<N>`` use the currently selected stateful pulse sequencer index.
For example, ``:SOURce:PULSe0:STATe ON`` operates on pulse sequencer 0, while
``:SOURce:PULSe:STATe ON`` operates on the currently selected pulse sequencer.


.. _scpi_pulse_select:

``:SELect``
===========

 | :SOURce:PULSe:SELect?
 | :SOURce:PULSe:SELect 0 | 1 | 2

This command selects the pulse sequencer index for stateful configuration
operations. The sequencer IDs reflect the pulse sequencer index where 0 is pulse
sequencer 0 and so on.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS:SEL 0
   :SOUR:PULS:SEL?
   >>> 0

.. note::
 * \*RST resets ``:SOURce:PULSe:SELect`` to `0`.
 * Only the query command is intended to be used during device operation.


.. _scpi_pulse_state:

``:STATe``
===========

 | :SOURce:PULSe<N>:STATe?
 | :SOURce:PULSe<N>:STATe ON | OFF

This command enables or disables the pulse sequencer at sequencer ``<N>`` if
stated, or the selected sequencer if not. Pulse sequencers that are not enabled
will not be processed during program execution.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:STAT ON
   :SOUR:PULS0:STAT?
   >>> 1

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:STATe`` to `OFF`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_input:

``:INPut``
==========

 | :SOURce:PULSe<N>:INPut?
 | :SOURce:PULSe<N>:INPut 0 | 1 | 2

This command sets the clock input source of the pulse sequencer at sequencer
``<N>`` if stated, or the selected sequencer if not. The input IDs reflect the
clock sequencer index where 0 is clock sequencer 0 and so on. A pulse sequencer
requires a clock sequencer input to listen to; without one, the pulse sequencer
will never execute.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:INP 0
   :SOUR:PULS0:INP?
   >>> 0

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:INPut`` to `0`.
 * Configuration commands are not allowed during device operation.


.. _scpi_pulse_divider:

``:DIVider``
============

 | :SOURce:PULSe<N>:DIVider?
 | :SOURce:PULSe<N>:DIVider HIGH | HIGH_RES | MED | MED_RES | LOW | LOW_RES | VERY_LOW | VERY_LOW_RES | VERY_VERY_LOW | VERY_VERY_LOW_RES

This command sets the time scale of the pulse sequencer at sequencer ``<N>`` if
stated, or the selected sequencer if not. The string arguments are converted to
a clock divider that changes the clock resolution of the pulse sequencer. For
example, ``HIGH_RES`` results in a clock divider of `1`, while ``LOW_RES``
results in a clock divider of `5`.

.. csv-table:: Divider Data Description
   :header: "SCPI String", "Clock Divider", "Unit Time Scale"
   :widths: 25, 10, 20

   "``HIGH_RES`` or ``HIGH``", "1", "Clock resolution is 4 ns"
   "``MED_RES`` or ``MED``", "2", "Clock resolution is 8 ns"
   "``LOW_RES`` or ``LOW``", "5", "Clock resolution is 20 ns"
   "``VERY_LOW_RES`` or ``VERY_LOW``", "25", "Clock resolution is 100 ns"
   "``VERY_VERY_LOW_RES`` or ``VERY_VERY_LOW``", "250", "Clock resolution is 1,000 ns (1 us)"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:DIV MED_RES
   :SOUR:PULS0:DIV?
   >>> 2

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:DIVider`` to `HIGH_RES`.
 * Configuration commands are not allowed during device operation.
 * Cached pulse sequence data needs to be re-applied when the divider is changed.
 * Pulse sequencer clock dividers should generally match the clock dividers of the chosen clock sequencer.


.. _scpi_pulse_units:

``:UNITs``
==========

 | :SOURce:PULSe<N>:UNITs?
 | :SOURce:PULSe<N>:UNITs NS | US | MS | S | M | H

This command sets the data units of the pulse sequencer at sequencer ``<N>`` if
stated, or the selected sequencer if not. The string arguments are converted to
a unit scaler that converts the supplied delay values into nanoseconds before
conversion to clock cycles. For example, ``NS`` results in a unit scaler of `1`,
while ``US`` results in a unit scaler of `1.0e3` because there are 1,000
nanoseconds in one microsecond.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Unit Time Scale"
   :widths: 15, 25

   "``NS``", "Pulse delay data is in nanoseconds"
   "``US``", "Pulse delay data is in microseconds"
   "``MS``", "Pulse delay data is in milliseconds"
   "``S``", "Pulse delay data is in seconds"
   "``M``", "Pulse delay data is in minutes"
   "``H``", "Pulse delay data is in hours"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:UNIT US
   :SOUR:PULS0:UNIT?
   >>> 1000.0

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:UNITs`` to the default unit scale configured by the device.
 * Configuration commands are not allowed during device operation.
 * Cached pulse sequence data needs to be re-applied when data units are changed.


.. _scpi_pulse_data:

``:DATA``
=========

 | :SOURce:PULSe<N>:DATA?

This command queries the instruction buffer of the pulse sequencer at sequencer
``<N>`` if stated, or the selected sequencer if not. This is read only and cannot
be set directly. The returned data is the applied low-level instruction buffer,
not the temporary ``DATA:BUFFer`` cache.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:DATA?
   >>> 0,1,0,1,0,1...

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:DATA`` to the device default pulse instruction buffer.


.. _scpi_pulse_reset:

``:RESet``
==========

 | :SOURce:PULSe<N>:RESet

This command resets the pulse sequencer at sequencer ``<N>`` if stated, or the
selected sequencer if not.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:RES

.. note::
 - \*RST calls this command on all sequencers.
 - Command is not allowed during device operation.


=============================
``:DATA:BUFFer`` Properties
=============================
``DATA:BUFFer`` is a subdirectory that controls the static instruction buffers
used by the pulse sequencer SCPI interface. These buffers are intermediary and
are not applied to a pulse sequencer until the ``:APPly`` command is executed.

The output and delay buffers are separate, global cache buffers. The command
still accepts a pulse sequencer suffix so the selected pulse sequencer can be
validated and used during ``:APPly``. The output and delay buffers must contain
the same number of elements before ``:APPly`` is executed.


.. _scpi_pulse_data_buffer_output:

``:OUTPut``
===========

 | :SOURce:PULSe<N>:DATA:BUFFer:OUTPut?
 | :SOURce:PULSe<N>:DATA:BUFFer:OUTPut <list of uint32_t>

This command caches output-state instructions into the static internal output
buffer. This buffer is not applied to a pulse sequencer until
``:SOURce:PULSe<N>:DATA:BUFFer:APPly`` is executed.

Each output value represents the desired output state for one pulse instruction.
Only the supported output-state bits are valid; values outside the output mask
will raise a `data out of range` error during ``:APPly``.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:DATA:BUFF:OUTP 64,2048,512
   :SOUR:PULS0:DATA:BUFF:OUTP?
   >>> 64,2048,512,0,0,0...

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:DATA:BUFFer:OUTPut`` to all zeros.
 * Configuration commands are not allowed during device operation.
 * Cached parameters need to be applied to a pulse sequencer before they can be used.


.. _scpi_pulse_data_buffer_delay:

``:DELay``
==========

 | :SOURce:PULSe<N>:DATA:BUFFer:DELay?
 | :SOURce:PULSe<N>:DATA:BUFFer:DELay <list of doubles>

This command caches delay instructions into the static internal delay buffer.
This buffer is not applied to a pulse sequencer until
``:SOURce:PULSe<N>:DATA:BUFFer:APPly`` is executed.

Delay values are interpreted using the currently selected
``:SOURce:PULSe<N>:UNITs`` setting. During ``:APPly``, each delay is converted to
nanoseconds and then converted to clock cycles using the currently selected
``:SOURce:PULSe<N>:DIVider`` setting. Values that are invalid during the
conversion process will raise a `data out of range` error.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:DATA:BUFF:DEL 30.5,10.1,0.003
   :SOUR:PULS0:DATA:BUFF:DEL?
   >>> 30.5,10.1,0.003,0,0,0...

.. note::
 * \*RST resets ``:SOURce:PULSe<N>:DATA:BUFFer:DELay`` to all zeros.
 * Configuration commands are not allowed during device operation.
 * Cached parameters need to be applied to a pulse sequencer before they can be used.


.. _scpi_pulse_data_buffer_clear:

``:CLEar``
==========

 | :SOURce:PULSe<N>:DATA:BUFFer:CLEar

This command clears the cached output and delay buffers and sets them all to
zero.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:DATA:BUFF:CLE
   :SOUR:PULS0:DATA:BUFF:OUTP?
   >>> 0,0,0,0,0...

.. note::
 * \*RST calls this command.
 * Command is not allowed during device operation.


.. _scpi_pulse_data_buffer_apply:

``:APPly``
==========

 | :SOURce:PULSe<N>:DATA:BUFFer:APPly

This command loads the currently cached output and delay buffers into pulse
sequencer ``<N>`` if stated, or the selected sequencer if not. Delay values are
converted using the current pulse units and pulse divider before they are stored
in the applied pulse instruction buffer.

The output and delay buffers must contain the same number of values. If they do
not, the command raises a `lists not same length` error. Delay values that cannot
be converted into valid clock cycles, or output values that exceed the supported
output mask, raise a `data out of range` error.

The applied instruction buffer alternates output-state and delay-cycle entries.
Output instructions are stored at even indexes and delay instructions are stored
at odd indexes. The final two entries in the applied instruction buffer are used
as termination flags.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :SOUR:PULS0:DATA:BUFF:OUTP 64,2048,512
   :SOUR:PULS0:DATA:BUFF:DEL 30.5,10.1,0.003
   :SOUR:PULS0:DATA:BUFF:APP
   :SOUR:PULS0:DATA?
   >>> 64,7620,2048,2520,512,1...

.. note::
 * Command is not allowed during device operation.
 * Cached parameters are converted using the current data units and clock divider.
 * Cached output and delay buffers need to have the same number of values before applying.
