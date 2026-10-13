Python Installation
===================

OpenSync Python wheels will be included in the target archives attached to
`OpenSync GitHub releases <https://github.com/ErichZimmer/OpenSync/releases>`_.
Download a wheel that matches your Python version and platform. In the Python
environment where you intend to use OpenSync, install the downloaded file:

.. code-block:: console

   python -m pip install "./downloaded-wheel.whl"

Replace ``downloaded-wheel.whl`` with the wheel's actual filename. Once it is
installed, you can ``import opensync`` in scripts and notebooks using that
environment.

For an introduction to device control, see :doc:`Notebooks
<../basic_usage/notebooks>`. Function details are in the :doc:`Python API
Reference <../../api_reference/index>`.
