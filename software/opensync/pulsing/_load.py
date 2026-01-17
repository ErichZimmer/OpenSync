from ..communication import device_comm_write
from . import _buffer
from . import _conversion as _conv


__all__ = [
    'device_params_load'
]


def device_params_load(
    device: 'opensync',
    clock_params: list['clock_params'],
    pulse_params: list['pulse_params'],
    reset: bool=True
):
    if reset == True:
        _buffer.device_timing_reset(
            device
        )
    
    # Load all clock parameters
    for clk_param in clock_params:
        # Load instructions
        resp = _buffer._device_clock_inst_triggered_load(
            device,
            clk_param
        )
        
        # check for errors
        for msg in resp:
            if 'error' in msg.lower():
                return resp

        # Load instructions
        resp = _buffer._device_clock_inst_freerun_load(
            device,
            clk_param
        )
        
        # check for errors
        for msg in resp:
            if 'error' in msg.lower():
                return resp

        # Load clock configs
        resp = _buffer._device_clock_config_load(
            device,
            clk_param
        )

        # check for errors
        for msg in resp:
            if 'error' in msg.lower():
                return resp

    # Load all pulse parameters
    for pls_param in pulse_params:
        # Load instructions
        resp = _buffer._device_pulse_inst_load(
            device,
            pls_param
        )

        # check for errors
        for msg in resp:
            if 'error' in msg.lower():
                return resp

        # Load pulse configs
        resp = _buffer._device_pulse_config_load(
            device,
            pls_param
        )

        # check for errors
        for msg in resp:
            if 'error' in msg.lower():
                return resp

    return resp