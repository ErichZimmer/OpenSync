Device communication
====================

Discover a device, manage its serial connection, and query or change its state.
All functions are available from the ``opensync`` package.

.. currentmodule:: opensync

Connections
-----------

.. autofunction:: device_comm_search

.. autofunction:: device_comm_open

.. autofunction:: device_comm_close

.. autofunction:: device_comm_managed

.. autofunction:: device_comm_write

Device information and operation
--------------------------------

.. autofunction:: device_system_version

.. autofunction:: device_system_status

.. autofunction:: device_system_fire

.. autofunction:: device_system_stop
