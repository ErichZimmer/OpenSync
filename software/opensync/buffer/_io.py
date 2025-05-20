from ..communication._io import EOL, device_comm_write, _get_response, _parse_response 


__all__ = [
    'device_buffer_load',
    'device_buffer_clear',
    'device_buffer_size',
    'device_buffer_dump'
]


def device_buffer_load(
    device: 'opensync',
    output_state: list[int], 
    output_delay: list[int]
) -> list[str]:
    """Load a list of output states and delays to an OpenSync device.

    This function communicates with an OpenSync device over USB using serial
    communication. It sends a list of output states and corresponding delays
    to the device, which are used to control the device's operation. The
    function ensures that the lengths of the output states and delays are
    the same before proceeding with the communication.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    output_state : list[int]
        A list of integers representing the output states to be loaded into
        the device. Each integer corresponds to a specific state for the
        synchronizer output.
    output_delay : list[int]
        A list of integers representing the delays (in cycles) associated
        with each output state. The length of this list must match the length
        of the output_state list.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the load command. Each string corresponds to a 
        response from the device.

    Raises
    ------
    ValueError
        If the lengths of the output_state and output_delay lists do not
        match.

    Notes
    -----
    - The function sends a 'load' command to the device followed by pairs
      of output states and delays. After sending the commands, it sends an 
      'exit' command to terminate the communication.
    - The function currently skips certain checks for faster load times,
      which may need to be addressed in future implementations.
    - If an error occurs during the command execution, the command may
      register as invalid.

    """
    state_len = len(output_state)
    delay_len = len(output_delay)
    
    if state_len != delay_len:
        raise ValueError(
            f'Output state and delay size mismatch: {state_len} vs {delay_len}'
        )
        
    # Skip checks for faster load times
    device.write(('load' + EOL).encode())
    for bit, cycles in zip(output_state, output_delay):
      device.write((f'{bit} {cycles}' + EOL).encode())

    # TODO: fix? 
    # If an error occurs, this command registers as invalid.
    device.write(('exit' + EOL).encode())

    return _parse_response(_get_response(device))


def device_buffer_clear(device: 'opensync') -> list[str]:
    """Clear the device buffer of the OpenSync device.

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
    command = 'uset' # uset is short for unset
    return device_comm_write(device, command)


def device_buffer_size(device: 'opensync') -> list[str]:
    """Retrieve the size of the device buffer from the OpenSync device.

    This function sends a command to the OpenSync device to get the current
    size of its buffer.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the size command.
    """
    command = 'size'
    return device_comm_write(device, command)


def device_buffer_dump(device: 'opensync') -> list[str]:
    """Dump the contents of the device buffer from the OpenSync device.

    This function sends a command to the OpenSync device to retrieve the
    current contents of its buffer.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the dump command.
    """
    command = 'dump'
    return device_comm_write(device, command)