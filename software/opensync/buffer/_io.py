from ..communication._io import device_comm_write
from ..pulsing._conversion import convert_clock_inst
from ..pulsing._clock_container import MAX_SKIPS


__all__ = [
    'device_clock_inst_freerun_load',
    'device_clock_inst_triggered_load'
    'device_clock_reset',
    'device_buffer_pulse_load',
    'device_pulse_reset'
]


def device_clock_inst_freerun_load(
    device: 'opensync',
    clock_params: 'clock_params'
) -> list[str]:
    """Load a list of trps and delays to an OpenSync device.

    This function communicates with an OpenSync device over USB using serial
    communication. It sends a list of reps and corresponding delays to the
    device, which are used to control the device's operation. The
    function ensures that the lengths of the output states and delays are
    the same before proceeding with the communication.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.

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
    reps_inst, delay_inst = convert_clock_inst(clock_params)

    # Validate that reps and delays have equal length for variable timing
    reps_len = len(reps_inst)
    delay_len = len(delay_inst)
    
    if reps_len != delay_len:
        msg = f'Reps and delay size mismatch: {reps_len} vs {delay_len}'
        raise ValueError(msg)

    # Open access to instruction buffer
    device_comm_write(
        device,
        f'cldi {clock_id}'
    )

    for reps, cycles in zip(reps_inst, delay_inst):
        resp = device_comm_write(
            device,
            f'{reps} {cycles}'
        )

        # If an invalid response is returned, halt and return that response
        if 'Invalid' in resp[0]:
            return resp
        
    resp = device_comm_write(
        device,
        'exit'
    )

    return resp


def device_clock_inst_triggered_load(
    device: 'opensync',
    clock_params: 'clock_params'
) -> list[str]:
    """Load external trigger config to an OpenSync device.

    This function communicates with an OpenSync device over USB using serial
    communication. It sends the external trigger config uration to the device,
    which is used to control the device's operation.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the load command. Each string corresponds to a 
        response from the device.

    """
    clock_id = clock_params['clock_id']

    # Convert delay instructions to cycles in ns
    _, _, trigger_delay_inst = convert_clock_inst(clock_params)

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

    if 'Invalid' in resp[0]:
        return resp
    
    # Now load reps
    resp = device_comm_write(
        device,
        f'treps {clock_id} {trigger_reps}'
    )
    
    return resp


def device_clock_reset(
    device: 'opensync',
    clock_params: 'clock_params'
    ) -> list[str]:
    """Clear the clock state of the OpenSync device.

    This function sends a command to the OpenSync device to clear its buffer.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the clear command.
    """
    clock_id = clock_params['clock_id']

    command = f'crst {clock_id}' 
    return device_comm_write(device, command)