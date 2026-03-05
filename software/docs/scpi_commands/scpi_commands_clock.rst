.. _api_scpi_clock_reference:

================================
``CLOCk`` Submodule Reference
================================

Clock sequencer properties of an OpenSync device can be accessed using the ``CLOCk`` root path. Clock sequencer configuration and information can be set and queried when the device is in idle and not currently executing a program.


.. _scpi_clock_select:

``:SELect``
===========


 | :CLOCk:SELect?
 | :CLOCk:SELect 0 | 1 | 2 

This command selects the clock sequencer index for stateful configuration operations. The sequencer IDs reflect the clock sequencer index where 0 is clock sequencer 0 and so on.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :CLOC:SEL 0
   :CLOC:SEL?
   >>> 0

.. note::
 * \*RST resets ``:CLOCk:SELect`` to `0`
 * Only query command will be allowed during device operation.


.. _scpi_clock_state:

``:STATe``
===========


 | :CLOCk:STATe?
 | :CLOCk:STATe ON | OFF

This command enables or disables the clock sequencer at sequencer <N> if stated or selected sequencer if not. Clock sequencers that are not enabled will not be processed during program execution.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :CLOC0:STAT ON
   :CLOC0:STAT?
   >>> 1

.. note::
 * \*RST resets ``:CLOCk:STATe`` to `0`
 * Only query command will be allowed during device operation.


.. _scpi_clock_units:

``:UNITs``
==========

 | :CLOCk<N>:UNITs?
 | :CLOCk<N>:UNITs HERTz | KILOhertz | MEGAhertz

This command sets the units of the clock sequencer at sequencer <N> if stated or selected sequencer if not. The string arguments are converted to a unit scaler that converts nanoseconds to the selected data unit in regards to hertz. For example, ``HERTz`` would result in a unit scaler of `1.0` while ``KILOhertz`` would result in a unit scaler of `1.0e3` since one kilohertz is 1,000 hertz.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Unit Time Scale"
   :widths: 15, 15

   "``hertz`` or ``hert``", "Clock data is in the *hertz* time scale"
   "``kilohertz`` or ``kilo``", "Clock data is in the *kilohertz* time scale"
   "``megahertz`` or ``mega``", "Clock data is in the *megahertz* time scale"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC0:UNIT KILO
   CLOC0:UNIT?
   >>> 1.0e3

.. note::
 * \*RST resets ``:CLOCk<N>:UNITs`` to `HERTz`
 * Only query command will be allowed during device operation.
 * Cached/stored clock sequences need to be re-applied when data units are changed.


.. _scpi_clock_divider:

``:DIVider``
============

 | :CLOCk<N>:DIVider?
 | :CLOCk<N>:DIVider HIGH_res | MED_res | LOW_res | VERY_LOW_res | VERY_VERY_LOW_res

This command sets the time scale of the clock sequencer at sequencer <N> if stated or selected sequencer if not. The string arguments are converted to a clock divider that changes the clock resolution of the clock sequencer. For example, ``HIGH_res`` would result in a clock divider of `1` while ``LOW_res`` would result in a clock divider of `25`.

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

   CLOC0:DIV MED_RES
   CLOC0:DIV?
   >>> 2

.. note::
 * \*RST resets ``:CLOCk<N>:DIVider`` to `HIGH_res`
 * Only query command will be allowed during device operation.
 * Cached/stored clock sequences need to be re-applied when data units are changed.
 * Clock sequencer clock dividers should generally match the clock dividers of the chosen clock sequencer.


.. _scpi_clock_data:

``:DATA``
=========


 | :CLOCk:DATA?

This command queries the instruction buffer of the clock sequencer at sequencer <N> if stated or selected sequencer if not. This is read only and cannot be set directly.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :CLOC0:DATA?
   >>> 0,0,0,0,0,0...

.. note::
 * \*RST resets ``:CLOCk:DATA`` to all zeros.


.. _scpi_clock_reset:

``:Reset``
==========

 | :CLOCk<N>:RESet

This command resets the clock sequencer at sequencer <N> if stated or selected sequencer if not.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   :CLOCk:Reset

.. note::
 - \*RST calls this command on all sequencers.
 - Command is not allowed during device operation.


=======================
``:TRIGger`` Properties
=======================
``TRIGger`` is a subdirectory that controls the clock sequencer operation and how a fire signal is sent to a pulse sequencer listening to that clock id. 


.. _scpi_clock_trigger_delay_units:

``:UNITs``
==========

 | :CLOCk<N>:TRIGger:UNITs?
 | :CLOCk<N>:TRIGger:UNITs NANOsecond | MICROsecond | MILLIsecond | SECond | MINute | HOUR

This command sets the units of the clock sequencer's trigger system at sequencer <N> if stated or selected sequencer if not. The string arguments are converted to a unit scaler that converts nanoseconds to the selected data unit. For example, ``nanosecond`` would result in a unit scaler of `1` while ``microsecond`` would result in a unit scaler of `1.0e3` since there are 1,000 nanoseconds in one microsecond.

.. csv-table:: Units Data Description
   :header: "SCPI String", "Unit Time Scale"
   :widths: 15, 15

   "``nanosecond`` or ``nano``", "Trigger data is in *nanoseconds* time scale"
   "``microsecond`` or ``micro``", "Trigger data is in *microseconds* time scale"
   "``millisecond`` or ``milli``", "Trigger data is in *milliseconds* time scale"
   "``second`` or ``sec``", "Trigger data is in *seconds* time scale"
   "``minutes`` or ``min``", "Trigger data is in *minutes* time scale"

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC0:TRIG:UNIT MILLI
   CLOC0:TRIG:UNIT?
   >>> 1.0e6

.. note::
 * \*RST resets ``:CLOCk<N>:TRIGger:UNITs`` to `MICROsecond`
 * Only query command will be allowed during device operation.
 * Clock sequencer trigger data need to be re-applied when data units are changed.


.. _scpi_clock_trigger_mode:

``:MODe``
============

 | :CLOCk<N>:TRIGger:MODe?
 | :CLOCk<N>:TRIGger:MODe INTernal | RISING_EDGE

This command sets the clock operation mode of the sequencer. This allows for the sequencer to either trigger internally (software trigger) or listen to external events (external trigger).

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC0:TRIG:MODE INT
   CLOC0:TRIG:MODE?
   >>> 0

.. note::
 * \*RST resets ``:CLOCk<N>:TRIGger:MODe`` to `INTernal`
 * Only query command will be allowed during device operation.


.. _scpi_clock_trigger_data:

``:DATA``
============

 | :CLOCk<N>:TRIGger:DATA?
 | :CLOCk<N>:TRIGger:DATA int32_t,double,int32_t

This command sets the external trigger behavior using a three element list which contains the number of triggers to skip, the delay from external trigger to internal trigger signal, and the amount of resets to peform. The trigger delay is calculated by using the selected units and clock divider and needs to be re-applied if these settings are changed.  

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC0:TRIG:DATA 7,0.0,10
   CLOC0:TRIG:DATA?
   >>> 7,0.0,10

.. note::
 * \*RST resets ``:CLOCk<N>:TRIGger:DATA`` to all zeros.
 * Only query command will be allowed during device operation.
 * Clock sequencer trigger data need to be re-applied when data units are changed.


==========================
``:DATA:STOre`` Properties
==========================
``DATA:STOre`` is a subdirectory that controls the static instruction buffers within the clock sequencer. These buffers are intermediary and are not applied to a clock sequencer until the ``:APPly <N>`` command is executed.


.. _scpi_clock_data_store_freqs:

``:FREQs``
==========

 | :CLOCk:DATA:STOre:FREQs?
 | :CLOCk:DATA:STOre:FREQs <list of doubles>

This command caches output instructions into a static (global) internal buffer. This buffer is not assigned to any clock sequencer and exists in its own state.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC:DATA:STO:FREQ 7.662,15
   CLOC:DATA:STO:FREQ?
   >>> 7.662,15.0,0,0,0...

.. note::
 * \*RST resets ``:CLOCk:DATA:STOre:FREQs`` to all zeros
 * Only query command will be allowed during device operation.
 * Cached parameters need to be applied to a clock sequencer before they can be used.


.. _scpi_clock_data_store_reps:

``:REPs``
=========

 | :CLOCk:DATA:STOre:REPs?
 | :CLOCk:DATA:STOre:REPs <list of uint32_t>

This command caches delay instructions into a static (global) internal buffer. This buffer is not assigned to any clock sequencer and exists in its own state.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC:DATA:STO:REP 5,10
   CLOC:DATA:STO:REP?
   >>> 5,10,0,0,0...

.. note::
 * \*RST resets ``:CLOCk:DATA:STOre:REPs`` to all zeros
 * Only query command will be allowed during device operation.
 * Cached parameters need to be applied to a clock sequencer before they can be used.


.. _scpi_clock_data_store_clear:

``:CLEar``
==========

 | :CLOCk:DATA:STOre:CLEar

This command clears the cached output and delay buffers and sets them all to zero.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC:DATA:STO:CLE
   CLOC:DATA:STO:OUTP?
   >>> 0,0,0,0,0...

.. note::
 * \*RST calls this command.
 * Command is not allowed during device operation.


.. _scpi_clock_data_store_apply:

``:APPly``
==========

 | :CLOCk:DATA:STOre:APPly <N>

This command loads the currently cached frequency and repetition buffers into clock sequencer <N> if stated or selected sequencer if not. Frequency values are also converted to nanoseconds and then clock cycles with the currently selected units and clock divider. Values that are invalid during the conversion process will raise a `data out of range` error.

Examples
--------
.. code-block:: none
   :caption: Example SCPI code

   CLOC:DATA:STO:APP 0

.. note::
 * Command is not allowed during device operation.
 * Cached parameters are converted using the current data units and will error if values are out of range.
