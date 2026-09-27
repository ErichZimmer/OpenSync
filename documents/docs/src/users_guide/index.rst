Users Guide
===========

OpenSync is an open source synchronizer for particle image velocimetry (PIV)
and other laboratory measurements that depend on coordinated timing. Cameras,
pulsed illumination sources, and acquisition equipment must respond at known
times for their measurements to be useful. OpenSync provides a shared timing
reference and eight independently programmable output channels, allowing an
experiment's timing to be configured through a common device interface.

The project began with a practical barrier identified in the SDLC briefing
and feasibility study: commercial synchronizers can represent a substantial
cost for laboratories with limited funding. Simple custom microcontroller
solutions can reduce that cost, but often offer limited timing flexibility
or require firmware changes whenever an experiment's timing changes. OpenSync
exists to make a configurable synchronizer more accessible while keeping its
hardware and software available for inspection, modification, and reuse.

The design uses the Raspberry Pi RP2350's programmable input/output (PIO)
state machines to handle timing independently of the main processor's command
handling. The internal rate timer and individual output sequencers provide
configurable timing, synchronization, triggering, and gating. A USB SCPI
interface, Python library, and browser application let users change settings
without rewriting the timing firmware. The emphasis is on an affordable,
constructible device that supports a broad range of PIV measurements, rather
than reproducing every feature of a commercial instrument.

Hardware, firmware, and software
--------------------------------

.. toctree::
   :maxdepth: 1

   hardware
   firmware
   software

The Hardware Guide is reserved for building the physical device. The Firmware
Guide explains the code running on it, including release installation and
source compilation. The Software Guide covers Python, the PWA, and the
basic-operation notebooks.

Project background
------------------

This overview draws on the project's `requirements briefing
<https://github.com/ErichZimmer/OpenSync/blob/main/documents/SDLC/1_initalization/briefing.md>`_,
`feasibility study
<https://github.com/ErichZimmer/OpenSync/blob/main/documents/SDLC/1_initalization/feasibility.md>`_,
and `firmware revision planning
<https://github.com/ErichZimmer/OpenSync/blob/main/documents/SDLC/4_development/firmware_revision_planning.md>`_.
These SDLC documents record the design's evolution; early proposed channel
counts and configurations may differ from the current device.
