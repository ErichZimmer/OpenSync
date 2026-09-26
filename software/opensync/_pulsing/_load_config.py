from . import _load_buffer


__all__ = [
    'device_params_load'
]


def device_params_load(
    device: 'opensync',
    clock_params: dict,
    pulse_params: dict,
    reset: bool = True
) -> list[str]:
    if reset == True:
        resp = _load_buffer.device_timing_reset(
            device
        )

        for msg in resp:
            if 'error' in msg.lower():
                return resp

    # Load clock configuration first.
    resp = _load_buffer._device_clock_config_load(
        device,
        clock_params
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load each pulse channel configuration.
    for channel in pulse_params:
        channel_id = int(channel.split('_')[1])

        resp = _load_buffer._device_pulse_config_load(
            device,
            pulse_params[channel],
            channel_id
        )

        for msg in resp:
            if 'error' in msg.lower():
                return resp

    return resp