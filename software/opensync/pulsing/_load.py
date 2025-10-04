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
):
    # Load all clock parameters
    for clk_param in clock_params:
        # Load instructions
        if clk_param['ext_trigger'] == 'enabled':
            resp = _buffer.device_clock_inst_triggered_load(
                device,
                clk_param
            )
        else:
            resp = _buffer.device_clock_inst_freerun_load(
                device,
                clk_param
            )

        # check for errors
        if 'ok' not in resp[0].lower():
            return resp

    # Load all pulse parameters
    for pls_param in pulse_params:
        # Load instructions
        resp = _buffer.device_pulse_inst_load(
            device,
            pls_param
        )

        # check for errors
        if 'ok' not in resp[0].lower():
            return resp

    return resp