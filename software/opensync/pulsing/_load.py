from ..communication import device_comm_write
from ..buffer import device_buffer_load
from . import _conversion as _conv


__all__ = [
    'device_params_load'
]


def device_params_load(
    pulse_params: 'pulse_params',
    clock_params: 'clock_params'
):
    raise Exception('Not implemented yet')