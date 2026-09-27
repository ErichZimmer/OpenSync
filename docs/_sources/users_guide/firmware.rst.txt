Firmware Guide
==============

The firmware runs on the Raspberry Pi Pico 2's RP2350 microcontroller. It
accepts SCPI commands over USB, stores system and channel settings, and
configures the PIO state machines that generate timing signals.

Install firmware from a release
-------------------------------

Compiled firmware will be included with the corresponding
`OpenSync GitHub release <https://github.com/ErichZimmer/OpenSync/releases>`_.
For normal installation, download the OpenSync **UF2** firmware asset attached
to the release you want to use. The release's source-code ZIP or tarball is
for building from source; it is not the file copied to the device for flashing.

Follow the flashing steps below once you have the UF2 file. Compiling the
firmware yourself is only needed when building a development checkout or
making changes to the source.

.. _opensync-flashing:

Flash the device
----------------

1. Close any PWA, Python session, or serial terminal connected to OpenSync,
   then unplug the device from USB.
2. Access the Pico 2's **BOOTSEL** button. Hold it while reconnecting USB,
   then release it when the bootloader drive appears.
3. Copy the release's UF2 file onto that drive. The Pico 2 bootloader normally
   identifies the drive as ``RP2350``.
4. Wait for the file copy to finish. The device reboots and the bootloader
   drive disappears.
5. Reconnect through the PWA or Python library and query ``*IDN?`` and
   ``:DEVice:STATus?`` to check device identity and state.

Raspberry Pi documents this process in its
`Pico-series SDK and UF2 instructions
<https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html>`_.

How the firmware is organized
-----------------------------

The current application is in ``firmware/opensync``. The older prototype
projects elsewhere in ``firmware`` are separate from this build.

.. list-table:: Firmware source layout
   :header-rows: 1
   :widths: 30 70

   * - Section
     - Responsibility
   * - ``src/main.c``
     - Starts the application by calling ``core_2_init()``.
   * - ``src/system/core_2.c``
     - Initializes the clock, status, USB serial transport, and SCPI interface.
       Launches the sequencer core and processes incoming commands.
   * - ``src/system/core_1.c``
     - Loads PIO programs, configures enabled sequencers, starts execution,
       waits for completion or an abort, and releases timing resources.
   * - ``src/serial/``
     - Defines the SCPI command interface, parameter parsing, validation,
       error handling, and device/system/sequencer callbacks.
   * - ``src/sequencer/``
     - Configures the internal clock, T0 event timer, external trigger/gate
       inputs, and output channels, including their PIO and DMA resources.
   * - ``src/pio_assembly/``
     - Contains the low-level PIO programs for the internal clock, T0,
       and output-channel waveforms.
   * - ``src/structs/``
     - Defines the stored SCPI settings and the structures used to configure
       clock and pulse sequencers.
   * - ``src/status/``
     - Holds sequencer and debug status shared by the firmware's two cores.
   * - ``src/overclock/``
     - Sets the system clock used by the timing engine.
   * - ``src/usb_desc/`` and ``src/version/``
     - Provide USB descriptors and firmware identification information.
   * - ``external/scpi-parser/`` and ``external/prawn_do/``
     - Supply the SCPI parser and USB serial transport used by the application.
   * - ``CMakeLists.txt`` and ``src/CMakeLists.txt``
     - Select the Pico board and SDK, compile the application and dependencies,
       generate PIO headers, and produce the UF2 firmware.

During operation, USB commands update the SCPI settings. A start request tells
the sequencer core to prepare the internal clock, T0, and enabled channels.
PIO state machines perform the timing while DMA supplies their data. The
sequencer core monitors completion and stop requests, then cleans up the
resources. A requested stop can leave the device in ``ABORTED``; this state
is distinct from an unexpected program failure.

Build from source
-----------------

Tools and dependencies
~~~~~~~~~~~~~~~~~~~~~~

Use the `Raspberry Pi Pico extension for Visual Studio Code
<https://github.com/raspberrypi/pico-vscode>`_ to manage the SDK and toolchain,
or provide the equivalent CMake, Ninja, Arm compiler, and Pico SDK setup.
The project's CMake configuration currently selects ``pico2`` and records
Pico SDK ``2.1.1``, toolchain ``14_2_Rel1``, and picotool ``2.1.1`` for the
extension-managed setup.

The SCPI parser is a Git submodule. From the OpenSync repository root, fetch
the revision recorded by the checkout:

.. code-block:: console

   git submodule update --init --recursive firmware/opensync/external/scpi-parser

Using Visual Studio Code
~~~~~~~~~~~~~~~~~~~~~~~~

1. Open or import ``firmware/opensync`` as an existing Pico project using the
   Raspberry Pi Pico extension.
2. Allow its SDK/toolchain setup to finish and confirm the target is Pico 2.
3. Run the extension's **Compile** action for the ``opensync`` target.
4. Locate ``opensync.uf2`` in the build output and follow :ref:`opensync-flashing`.

Using a terminal
~~~~~~~~~~~~~~~~

With the Pico SDK and Arm toolchain available, set ``PICO_SDK_PATH`` to your
SDK directory. From the repository root:

.. code-block:: console

   cmake -S firmware/opensync -B firmware/opensync/build -G Ninja -DPICO_BOARD=pico2
   cmake --build firmware/opensync/build --target opensync

For this build layout, the firmware file is
``firmware/opensync/build/src/opensync.uf2``. Copy it to the bootloader drive
using the same flashing procedure as a release UF2. CMake also produces
other build artifacts, but the UF2 is the one used for drag-and-drop flashing.

For individual device commands and timing parameters, see the
:doc:`API Reference <../api_reference/index>`.
