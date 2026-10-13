Notebooks
=========

These notebooks introduce basic operation with the OpenSync Python package.
Use the Python environment where you installed the OpenSync wheel.

.. toctree::
   :maxdepth: 1

   ../../notebooks/device_communication_basics
   ../../notebooks/clock_setup
   ../../notebooks/pulse_timing
   ../../notebooks/scpi_pulse_basics

The documentation displays the notebooks and their saved outputs without
executing their cells or opening a hardware connection.

Connect and read device information
-----------------------------------

Connect OpenSync over USB, discover available devices, and read the identity
and status of the first one:

.. code-block:: python

   import opensync

   ports = opensync.device_comm_search()
   if not ports:
       raise RuntimeError("No OpenSync device found. Check the USB connection.")

   with opensync.device_comm_managed(ports[0]) as device:
       print(opensync.device_system_version(device))
       print(opensync.device_system_status(device))

The context manager closes the connection when the block ends. This example
reads device information without starting a timing sequence.

Configure and run a sequence
----------------------------

The normal Python workflow is:

1. Create clock and channel parameter dictionaries with ``get_clock_params()``
   and ``get_pulse_params()``.
2. Set timing, counters, synchronization, and enabled channels using the
   ``config_clock_*`` and ``config_pulse_*`` helper functions.
3. Inspect pulse timing with ``get_pulse_plot()`` when useful.
4. Open the serial connection and upload both configurations with
   ``device_params_load(device, clock_params, pulse_params)``.
5. Use ``device_system_fire()`` to start and ``device_system_stop()`` to stop.

The notebooks above work through these steps. Function details are in the
:doc:`Python API Reference <../../api_reference/index>`.
