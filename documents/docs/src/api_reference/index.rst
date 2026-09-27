API Reference
=============

This reference covers the OpenSync Python package and the device's SCPI
command interface. For installation and basic-operation notebooks, see the
:doc:`Software Guide <../users_guide/software>`.

Python API
----------

The pages below describe functions exported by ``opensync``. Their signatures
and docstrings are generated from the Python library in this repository.

.. toctree::
   :maxdepth: 1

   ../modules/communication
   ../modules/config_clock
   ../modules/config_pulse
   ../modules/loading

SCPI API
--------

SCPI provides command and query access over the USB serial interface. The
internal rate timer is ``PULSe0`` (T0); outputs A through H are addressed as
``PULSe1`` through ``PULSe8``. The current detailed reference covers the pulse
sequencer commands, including clock timing, buffers, counters, triggering,
and gating.

.. toctree::
   :maxdepth: 1

   ../scpi_commands/scpi_commands_pulse

Indexes
-------

* :ref:`genindex`
* :ref:`search`
