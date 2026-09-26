from .._communication._io import device_comm_write


__all__ = [
    '_device_clock_config_load',
    '_device_pulse_config_load',
    'device_clock_reset',
    'device_pulse_reset',
    'device_timing_reset'
]


def _device_clock_config_load(
    device: 'opensync',
    clock_params: dict
) -> list[str]:
    """Load the T0 clock configuration into an OpenSync device.

    Parameters
    ----------
    device : 'opensync'
        The OpenSync device that will receive the commands.
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.

    Returns
    -------
    response : list[str]
        The device response, including any reported errors.

    Notes
    -----
    - Configure a valid clock period before loading.
    - The period is already stored in seconds.
    - If an error occurs, loading stops and returns the response.

    """
    state        = int(clock_params['state'])
    divider      = clock_params['divider']
    period       = clock_params['period']
    bcounter     = clock_params['bcounter']
    pcounter     = clock_params['pcounter']
    ocounter     = clock_params['ocounter']
    trigger_mode = clock_params['trigger_mode']
    trigger_edge = clock_params['trigger_edge']
    gate_mode    = clock_params['gate_mode']
    gate_level   = clock_params['gate_level']

    # Load clock divider
    command = f':pulse0:divider {divider}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock period
    command = f':pulse0:period {period}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock B counter
    command = f':pulse0:bcounter {bcounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock P counter
    command = f':pulse0:pcounter {pcounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock O counter
    command = f':pulse0:ocounter {ocounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger mode
    command = f':pulse0:trigger:mode {trigger_mode}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger edge
    command = f':pulse0:trigger:edge {trigger_edge}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load gate mode
    command = f':pulse0:gate:mode {gate_mode}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load gate level
    command = f':pulse0:gate:logic {gate_level}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load the requested clock state
    command = f':pulse0:state {state}'
    resp = device_comm_write(
        device,
        command
    )

    return resp


def _device_pulse_config_load(
    device: 'opensync',
    pulse_params: dict,
    channel_id: int
) -> list[str]:
    """Load one pulse channel configuration into an OpenSync device.

    Parameters
    ----------
    device : 'opensync'
        The OpenSync device that will receive the commands.
    pulse_params : dict
        One channel's configuration, such as pulse_params['channel_1'].
    channel_id : int
        The firmware output channel number, from 1 through 8.

    Returns
    -------
    response : list[str]
        The device response, including any reported errors.

    Notes
    -----
    - Stored edge times are converted into SCPI state/duration pairs.
    - The dictionary's edge times are not modified.
    - Empty disabled channels do not send a buffer command.
    - If an error occurs, loading stops and returns the response.

    """
    state        = int(pulse_params['state'])
    divider      = pulse_params['divider']
    pulse_data   = pulse_params['data']
    wcounter     = pulse_params['wcounter']
    bcounter     = pulse_params['bcounter']
    pcounter     = pulse_params['pcounter']
    ocounter     = pulse_params['ocounter']
    sync         = pulse_params['sync']
    output_level = pulse_params['output_level']

    if not 1 <= channel_id <= 8:
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    pulse_len = len(pulse_data)

    if pulse_len not in (0, 2, 4):
        msg = f'Invalid pulse data size of {pulse_len}'
        raise ValueError(msg)

    if state and pulse_len == 0:
        msg = f'Pulse channel {channel_id} is enabled but has no pulse data'
        raise ValueError(msg)

    # Convert absolute edge times to state/duration pairs.
    pulse_inst = []
    previous_falling_edge = 0.0

    for i in range(0, pulse_len, 2):
        rising_edge = pulse_data[i]
        falling_edge = pulse_data[i + 1]

        delay = round(rising_edge - previous_falling_edge, 9)
        pulse_length = round(falling_edge - rising_edge, 9)

        # Omit the initial LOW interval when the first pulse starts at zero.
        if i > 0 or delay != 0:
            pulse_inst += ['OFF', str(delay)]

        pulse_inst += ['ON', str(pulse_length)]
        previous_falling_edge = falling_edge

    # Load pulse divider
    command = f':pulse{channel_id}:divider {divider}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load initial wait counter
    command = f':pulse{channel_id}:wcounter {wcounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse B counter
    command = f':pulse{channel_id}:bcounter {bcounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse P counter
    command = f':pulse{channel_id}:pcounter {pcounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse O counter
    command = f':pulse{channel_id}:ocounter {ocounter}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load synchronization source
    command = f':pulse{channel_id}:sync {sync}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load output level
    command = f':pulse{channel_id}:output:level {output_level}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse instructions
    if pulse_len > 0:
        pulse_inst_char = ','.join(pulse_inst)

        command = f':pulse{channel_id}:buffer {pulse_inst_char}'
        resp = device_comm_write(
            device,
            command
        )

        for msg in resp:
            if 'error' in msg.lower():
                return resp

    # Load the requested pulse channel state
    command = f':pulse{channel_id}:state {state}'
    resp = device_comm_write(
        device,
        command
    )

    return resp


def device_clock_reset(
    device: 'opensync'
) -> list[str]:
    """Reset the T0 clock configuration."""
    command = ':pulse0:reset'
    return device_comm_write(device, command)


def device_pulse_reset(
    device: 'opensync',
    channel_id: int
) -> list[str]:
    """Reset one output channel, numbered 1 through 8."""
    if not 1 <= channel_id <= 8:
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    command = f':pulse{channel_id}:reset'
    return device_comm_write(device, command)


def device_timing_reset(
    device: 'opensync'
) -> list[str]:
    """Reset T0 and all eight output channel configurations."""
    for channel_id in range(9):
        command = f':pulse{channel_id}:reset'
        resp = device_comm_write(
            device,
            command
        )

        for msg in resp:
            if 'error' in msg.lower():
                return resp

    return resp