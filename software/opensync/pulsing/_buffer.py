from ..communication._io import device_comm_write
from ._conversion import _convert_clock_inst, _convert_pulse_inst
from ._clock_container import MAX_SKIPS, VALID_CLOCK_IDS


__all__ = [
    '_device_clock_inst_freerun_load',
    '_device_clock_inst_triggered_load',
    '_device_clock_config_load',
    '_device_pulse_inst_load',
    '_device_pulse_config_load',
    'device_clock_reset',
    'device_pulse_reset',
    'device_timing_reset'
]


def _device_clock_inst_freerun_load(
    device: 'opensync',
    clock_params: 'clock_params'
) -> list[str]:
    """Load a list of reps and delays to an OpenSync device.

    This function communicates with an OpenSync device over USB using serial
    communication. It sends a list of reps and corresponding delays to the
    device, which are used to control the device's operation. The function
    ensures that the lengths of the repitions and delays are the same before
    proceeding with the communication.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    clock_params : dict
        A dictionary containing clock parameters from get_clock_params`.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the load command. Each string corresponds to a 
        response from the device.

    Notes
    -----
    - The function sends a 'load' command to the device followed by pairs
      of clock reps and delays. After sending the commands, it sends an 
      'exit' command to terminate the communication.
    - If an error occurs during the command execution, the command will exit
      prematurely and return the recieved error.

    """
    clock_id = clock_params['clock_id']

    # Convert delay instructions to cycles in ns
    reps_inst, delay_inst, _ = _convert_clock_inst(clock_params)

    # Validate that reps and delays have equal length for variable timing
    reps_len = len(reps_inst)
    delay_len = len(delay_inst)
    
    if reps_len != delay_len:
        msg = f'Reps and delay size mismatch: {reps_len} vs {delay_len}'
        raise ValueError(msg)

    # Open access to instruction buffer
    device_comm_write(
        device,
        f'cldi {clock_id}',
        output=False
    )

    for reps, cycles in zip(reps_inst, delay_inst):
        resp = device_comm_write(
            device,
            f'{reps} {cycles}'
        )

        # If an invalid response is returned, halt and return that response
        if 'ok' not in resp[0].lower():
            return resp
        
    resp = device_comm_write(
        device,
        'exit'
    )

    return resp


def _device_clock_inst_triggered_load(
    device: 'opensync',
    clock_params: 'clock_params'
) -> list[str]:
    """Load external trigger config to an OpenSync device.

    This function communicates with an OpenSync device over USB using serial
    communication. It sends the external trigger configuration to the device,
    which is used to control the device's operation.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    clock_params : dict
        A dictionary containing clock parameters from`get_clock_params`.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the load command. Each string corresponds to a 
        response from the device.

    """
    clock_id = clock_params['clock_id']

    # Convert delay instructions to cycles in ns
    _, _, trigger_delay_inst = _convert_clock_inst(clock_params)

    trigger_skips = clock_params['ext_trigger_skips']
    trigger_reps = clock_params['reps_iter'][0] # only use the first index for triggered reps

    # Validate trigger count    
    if trigger_skips > MAX_SKIPS:
        msg = 'Trigger skips size too high'
        raise ValueError(msg)

    # Open access to trigger buffer
    resp = device_comm_write(
        device,
        f'tldi {clock_id} {trigger_skips} {trigger_delay_inst}'
    )

    if 'ok' not in resp[0].lower():
        return resp
    
    # Now load reps
    resp = device_comm_write(
        device,
        f'treps {clock_id} {trigger_reps}'
    )
    
    return resp

def _device_clock_config_load(
    device: 'opensync',
    clock_params: 'clock_params'
) -> list[str]:
    """
    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    pulse_params : dict
        A dictionary containing pulse parameters from get_pulse_params`.

    Returns
    -------
    response : list[str]
    """
    clock_id = clock_params['clock_id']
    clock_divider = clock_params['clock_divider']
    trigger_id = clock_params['ext_trigger_id']
    clock_type = int(clock_params['ext_trigger'] == 'enabled')

    # Activate clock channel
    resp = device_comm_write(
        device,
        f'cact {clock_id} 1'
    )

    if 'ok' not in resp[0].lower():
        return resp
        
    # Load clock divider
    resp = device_comm_write(
        device,
        f'cdiv {clock_id} {clock_divider}'
    )

    if 'ok' not in resp[0].lower():
        return resp

    # Load external trigger pin
    resp = device_comm_write(
        device,
        f'tset {clock_id} {trigger_id}'
    )

    if 'ok' not in resp[0].lower():
        return resp

    # Load clock type
    resp = device_comm_write(
        device,
        f'ctyp {clock_type}'
    )    

    return resp


def _device_pulse_inst_load(
    device: 'opensync',
    pulse_params: 'pulse_params'
) -> list[str]:
    """Load a list of output states and delays to an OpenSync device.

    This function communicates with an OpenSync device over USB using serial
    communication. It sends a list of ouput state and corresponding delays to
    the device, which are used to control the device's operation.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    pulse_params : dict
        A dictionary containing pulse parameters from get_pulse_params`.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the load command. Each string corresponds to a 
        response from the device.

    Notes
    -----
    - The function sends a 'load' command to the device followed by pairs
      of output states and delays. After sending the commands, it sends an 
      'exit' command to terminate the communication.
    - If an error occurs during the command execution, the command will exit
      prematurely and return the recieved error.

    """
    pulse_id = pulse_params['pulse_id']
    clock_id = pulse_params['clock_id']
    clock_divider = pulse_params['clock_divider']

    # Convert delay instructions to cycles in ns
    output_inst, delay_inst = _convert_pulse_inst(pulse_params)

    # Validate that outputs and delays have equal length for variable timing
    output_len = len(output_inst)
    delay_len = len(delay_inst)
    
    if output_len != delay_len:
        msg = f'Reps and delay size mismatch: {output_len} vs {delay_len}'
        raise ValueError(msg)

    # Open access to instruction buffer
    device_comm_write(
        device,
        f'pldi {pulse_id}', 
        output=False
    )

    for output, cycles in zip(output_inst, delay_inst):
        resp = device_comm_write(
            device,
            f'{output} {cycles}'
        )

        # If an invalid response is returned, halt and return that response
        if 'ok' not in resp[0].lower():
            return resp
        
    resp = device_comm_write(
        device,
        'exit'
    )

    if 'ok' not in resp[0].lower():
        return resp

    # Now load clock trigger channel
    resp = device_comm_write(
        device,
        f'pset {pulse_id} {clock_id}'
    )

    if 'ok' not in resp[0].lower():
        return resp
    
    # Now load clock divider
    resp = device_comm_write(
        device,
        f'pdiv {pulse_id} {clock_divider}'
    )

    return resp


def _device_pulse_config_load(
    device: 'opensync',
    pulse_params: 'pulse_params'
) -> list[str]:
    """
    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    pulse_params : dict
        A dictionary containing pulse parameters from get_pulse_params`.

    Returns
    -------
    response : list[str]
    """
    pulse_id = pulse_params['pulse_id']
    trigger_id = pulse_params['clock_id']
    clock_divider = pulse_params['clock_divider']

    # Activate output pulse channel
    resp = device_comm_write(
        device,
        f'pact {pulse_id} 1'
    )
    
    if 'ok' not in resp[0].lower():
        return resp
        
    # Load clock divider
    resp = device_comm_write(
        device,
        f'pdiv {pulse_id} {clock_divider}'
    )

    if 'ok' not in resp[0].lower():
        return resp

    # Load clock trigger pin
    resp = device_comm_write(
        device,
        f'pset {pulse_id} {trigger_id}'
    )

    return resp


def device_clock_reset(
    device: 'opensync',
    clock_params: 'clock_params'
    ) -> list[str]:
    """Clear the clock state of the OpenSync device.

    This function sends a command to the OpenSync device to reset its buffer.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the reset command.
    """
    clock_id = clock_params['clock_id']

    command = f'crst {clock_id}' 
    return device_comm_write(device, command)

    
def device_pulse_reset(
    device: 'opensync',
    pulse_params: 'pulse_params'
    ) -> list[str]:
    """Clear the clock state of the OpenSync device.

    This function sends a command to the OpenSync device to reset its buffer.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the reset command.
    """
    pulse_id = pulse_params['pulse_id']

    command = f'prst {pulse_id}' 
    return device_comm_write(device, command)


def device_timing_reset(
    device: 'opensync'
    ) -> list[str]:
    """Clear all pulse and clock states of the OpenSync device.

    This function sends a command to the OpenSync device to reset its buffer.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the reset command.
    """
    resp = []
    # Reset all clock channels
    for channel_id in VALID_CLOCK_IDS:
        command = f'crst {channel_id}'
        resp.append(device_comm_write(device, command))

    # Reset all pulse channels
    for channel_id in VALID_CLOCK_IDS:
        command = f'prst {channel_id}'
        resp.append(device_comm_write(device, command))

    return resp