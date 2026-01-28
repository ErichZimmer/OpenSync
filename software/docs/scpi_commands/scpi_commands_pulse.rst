.. _api_scpi_pulse_reference:

================================
``PULSe`` Submodule Reference
================================

Pulse sequencer properties of an OpenSync device can be accessed using the ``PULSe`` root path. Pulse sequencer configuration and information can be set and queried when the device is in idle and not currently executing a program.


.. _scpi_pulse_select:

``:SELect``
===========


 | :PULSe:SELect?
 | :PULSe:SELect 0 | 1 | 2 

This command selects the pulse sequencer index for stateful configuration operations. The sequencer IDs reflect the pulse sequencer index where 0 is pulse sequencer 0 and so on.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS:SEL 0
   :PULS:SEL?
   >>> 0

.. note::
 * \*RST resets ``:PULSe:SELect`` to `0`
 * Only query command will be allowed during device operation.


.. _scpi_pulse_state:

``:STATe``
===========


 | :PULSe:STATe?
 | :PULSe:STATe ON | OFF

This command enables or disables the pulse sequencer at sequencer <N> if stated or selected sequencer if not. Pulse sequencers that are not enabled will not be processed during program execution.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:STAT ON
   :PULS0:STAT?
   >>> 1

.. note::
 * \*RST resets ``:PULSe:STATe`` to `0`
 * Only query command will be allowed during device operation.


.. _scpi_pulse_units:

``:UNITs``
==========

 | :PULSe<N>:UNITs?
 | :PULSe<N>:UNITs NANOsecond | MICROsecond | MILLIsecond | SECond | MINute | HOUR

This command sets the units of the pulse sequencer at sequencer <N> if stated or selected sequencer if not. The string arguments are converted to a unit scaler that converts nanoseconds to the selected data unit. For example, ``nanosecond`` would result in a unit scaler of `1` while ``microsecond`` would result in a unit scaler of `1.0e3` since there are 1,000 nanoseconds in one microsecond.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Unit Time Scale"
   :widths: 15, 15

   "``nanosecond`` or ``nano``", "Pulse data is in *nanoseconds* time scale"
   "``microsecond`` or ``micro``", "Pulse data is in *microseconds* time scale"
   "``millisecond`` or ``milli``", "Pulse data is in *milliseconds* time scale"
   "``second`` or ``sec``", "Pulse data is in *seconds* time scale"
   "``minutes`` or ``min``", "Pulse data is in *minutes* time scale"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   PULS0:UNIT MILLI
   PULS0:UNIT?
   >>> 1.0e6

.. note::
 * \*RST resets ``:PULSe<N>:UNITs`` to `MICROsecond`
 * Only query command will be allowed during device operation.
 * Cached/stored pulse sequences need to be re-applied when data units are changed.


.. _scpi_pulse_divider:

``:DIVider``
==========

 | :PULSe<N>:DIVider?
 | :PULSe<N>:DIVider HIGH_res| MED_res | LOW_res | VERY_LOW_res | VERY_VERY_LOW_res

This command sets the time scale of the pulse sequencer at sequencer <N> if stated or selected sequencer if not. The string arguments are converted to a clock divider that changes the clock resolution of the pulse sequencer. For example, ``HIGH_res`` would result in a clock divider of `1` while ``LOW_res`` would result in a clock divider of `25`.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Clock Divider", "Unit Time Scale"
   :widths: 15, 5, 15

   "``high_res`` or ``high``", "1", "Clock resolution is 4 ns"
   "``med_res`` or ``med``", "2", "Clock resolution is 8 ns"
   "``low_res`` or ``low``", "5", "Clock resolution is 20 ns"
   "``very_low_res`` or ``very_low``", "25", "Clock resolution is 100 ns"
   "``very_very_low_res`` or ``very_very_low``", "250", "Clock resolution is 1,000 ns (1 us)"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   PULS0:DIV MED_RES
   PULS0:DIV?
   >>> 2

.. note::
 * \*RST resets ``:PULSe<N>:DIVider`` to `HIGH_res`
 * Only query command will be allowed during device operation.
 * Cached/stored pulse sequences need to be re-applied when data units are changed.
 * Pulse sequencer clock dividers should generally match the clock dividers of the chosen clock sequencer.


.. _scpi_pulse_pin:

``:PIN``
========


 | :PULSe<N>:PIN?
 | :PULSe<N>:PIN 0 | 1 | 2 

This command sets the clock signal pin of the pulse sequencer at sequencer <N> if stated or selected sequencer if not. The pin IDs reflect the clock sequencer index where 0 is clock sequencer 0 and so on. A pulse sequencer requires a clock sequencer to listen to as without one, the pulse sequencer will never execute.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:PIN 0
   :PULS0:PIN?
   >>> 0

.. note::
 * \*RST resets ``:PULSe<N>:PIN`` to `0`
 * Only query command will be allowed during device operation.


.. _scpi_pulse_data:

``:DATA``
=========


 | :PULSe:DATA?

This command queries the instruction buffer of the pulse sequencer at sequencer <N> if stated or selected sequencer if not. This is read only and cannot be set directly.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULS0:DATA?
   >>> 0,1,0,1,0,1...

.. note::
 * \*RST resets ``:PULSe:DATA`` to al zeros.


.. _scpi_pulse_reset:

``:Reset``
===========

 | :PULSe<N>:RESet

This command resets the pulse sequencer at sequencer <N> if stated or selected sequencer if not.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :PULSe:Reset

.. note::
 - \*RST calls this command on all sequencers.
 - Command is not allowed during device operation.


==========================
``:DATA:STOre`` Properties
==========================
``DATA:STOre`` is a subdirectory that controls the static instruction buffers within the pulse sequencer. These buffers are intermediary and are not applied to a pulse sequencer until the ``:APPly <N>`` command is executed.


.. _scpi_pulse_data_store_outputs:

``:OUTPuts``
============

 | :PULSe:DATA:STOre:OUTPuts?
 | :PULSe:DATA:STOre:OUTPuts <list of uint32_t>

This command caches output instructions into a static (global) internal buffer. This buffer is not assigned to any pulse sequencer and exists in its own state.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   PULS:DATA:STO:OUTP 64,2048,512
   PULS:DATA:STO:OUTP?
   >>> 64,2048,512

.. note::
 * \*RST resets ``:PULSe:DATA:STOre:OUTPuts`` to all zeros
 * Only query command will be allowed during device operation.
 * Cached parameters need to be applied to a pulse sequencer before they can be used.


.. _scpi_pulse_data_store_delays:

``:DELays``
===========

 | :PULSe:DATA:STOre:DELays?
 | :PULSe:DATA:STOre:DELays <list of doubles>

This command caches delay instructions into a static (global) internal buffer. This buffer is not assigned to any pulse sequencer and exists in its own state.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   PULS:DATA:STO:DEL 30.5,10.1,0.003
   PULS:DATA:STO:DEL?
   >>> 30.5,10.1,0.003

.. note::
 * \*RST resets ``:PULSe:DATA:STOre:DELays`` to all zeros
 * Only query command will be allowed during device operation.
 * Cached parameters need to be applied to a pulse sequencer before they can be used.


.. _scpi_pulse_data_store_clear:

``:CLEar``
==========

 | :PULSe:DATA:STOre:CLEar

This command clears the cached output and delay buffers and sets them all to zero.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   PULS:DATA:STO:CLE
   PULS:DATA:STO:OUTP?
   >>> 0,0,0,0,0...

.. note::
 * \*RST calls this command.
 * Command is not allowed during device operation.


.. _scpi_pulse_data_store_apply:

``:APPly``
==========

 | :PULSe:DATA:STOre:APPly <N>

This command loads the currently cached output and delay buffers into pulse sequencer <N> if stated or selected sequencer if not. Delay values are also converted to nanoseconds and then clock cycles with the currently selected units and clock divider. Values that are invalid during the conversion process will raise a `data out of range` error.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   PULS:DATA:STO:APP 0

.. note::
 * Command is not allowed during device operation.
 * Cached parameters are converted using the current data units and will error if values are out of range.