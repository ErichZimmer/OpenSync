Upload settings
===============

.. currentmodule:: opensync

Create and edit clock and pulse parameter dictionaries before uploading them.
``device_params_load`` sends the clock configuration first, then each pulse
channel. Its ``reset`` argument controls whether device timing is reset before
loading the settings. Uploading parameters does not start a run; use
:func:`device_system_fire` when the device is ready.

.. autofunction:: device_params_load
