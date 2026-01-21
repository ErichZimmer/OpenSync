.. _scpi_pulse_units:

:PULSe<N>:UNITs
===============

```
:PULSe<N>:UNITs?
:PULSe<N>:UNITs NANOsecond | MICROsecond | MILLIsecond | SECond | MINute | HOUR
```

This command sets the units of the instruction cache buffer at pulse sequencer <N>. The instruction cache buffer is automatically converted to clock cycles when it is stored in the pulse sequencer and can be applied by the `:PULSe<N>:INSTructions:STOre:APPly` command.

Examples
--------

`:PULS0:UNIT MILLI`
`:PULS0:UNIT?`

Notes
-----
 - \*RST resets :PULSe<N>:UNITs to `MICROsecond`
 - Only query command will be allowed during device operation.
 - Cached and stored pulse sequences need to be reloaded into cache and applied when data units are changed.


.. _scpi_pulse_pin:

:PULSe<N>:PIN
=============

```
:PULSe<N>:PIN?
:PULSe<N>:PIN 0 | 1 | 2 
```

This command sets the clock signal pin of the instruction cache buffer at pulse sequencer <N>. 

Examples
--------

`:PULS0:PIN 0`
`:PULS0:PIN?`

Notes
-----
 - \*RST resets :PULSe<N>:PIN to `0`
 - Only query command will be allowed during device operation.


.. _scpi_pulse_data_store_outputs:

:PULSe:DATA:STOre:OUTPuts
=========================

```
:PULSe:DATA:STOre:OUTPuts?
:PULSe:DATA:STOre:OUTPuts <uint32_t list>
```

This command caches output instructions into a static (global) internal buffer. This buffer is not assigned to any pulse sequencer and exists in its own state.

Examples
--------

`PULS:DATA:STO:OUTP 64,2048,512`
`PULS:DATA:STO:OUTP?`

Notes
-----
 - \*RST resets :PULSe:INSTructions:STOre:OUTPuts to all zeros
 - Only query command will be allowed during device operation.
 - Cached parameters need to be applied to a pulse sequencer before they can be used.


.. _scpi_pulse_data_store_delays:

:PULSe:DATA:STOre:DELays
========================

```
:PULSe:DATA:STOre:DELays?
:PULSe:DATA:STOre:DELays <double list>
```

This command caches delay instructions into a static (global) internal buffer. This buffer is not assigned to any pulse sequencer and exists in its own state.

Examples
--------

`PULS:DATA:STO:DEL 30.5,10.1,0.003`
`PULS:DATA:STO:DEL?`

Notes
-----
 - \*RST resets :PULSe:INSTructions:STOre:DELays to all zeros
 - Only query command will be allowed during device operation.
 - Cached parameters need to be applied to a pulse sequencer before they can be used.


.. _scpi_pulse_data_store_apply:

:PULSe<N>:DATA:STOre:APPly
==========================

```
:PULSe<N>:DATA:STOre:APPly
```

This command loads the currently cached output and delay buffers into pulse sequencer <N>. Delay values are also converted to nanoseconds and then clock cycles with the currently selected units. Values that are invalid during the conversion process will raise a data out of range error.

Examples
--------

`:PULS0:UNIT MILLI`
`:PULS0:UNIT?`

Notes
-----
 - \*RST resets :PULSe:INSTructions:STOre:DELays to all zeros
 - Only query command will be allowed during device operation.
 - Cached parameters are converted using the current data units and will error if values are out of range.