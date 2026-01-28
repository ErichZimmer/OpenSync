from .._communication._io import device_comm_write
from ._conversion import _convert_pulse_inst
from ._container_clock import MAX_SKIPS, VALID_CLOCK_IDS


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
    reps_inst = clock_params['reps_iter']
    freq_inst = clock_params['reps_freq']

    # Validate that reps and delays have equal length for variable timing
    reps_len = len(reps_inst)
    freq_len = len(freq_inst)
    
    if reps_len != freq_len:
        msg = f'Reps and delay size mismatch: {reps_len} vs {freq_len}'
        raise ValueError(msg)

    # Send information to clock data buffer
    reps_inst_char = ",".join(map(str, reps_inst))

    command = f':clock:data:store:reps {reps_inst_char}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp
            
    # Send information to clock data buffer (for freqs this time)
    freq_inst_char = ",".join(map(str, freq_inst))
    
    command = f':clock:data:store:freqs {freq_inst_char}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp
    
    # Now apply the current setting to the clock sequencer
    command = f':clock:data:store:apply {clock_id}'
    resp = device_comm_write(
        device,
        command
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

    trigger_delay = clock_params['ext_trigger_delay']
    trigger_skips = clock_params['ext_trigger_skips']
    trigger_reps = clock_params['reps_iter'][0] # only use the first index for triggered reps

    # Validate trigger count    
    if trigger_skips > MAX_SKIPS:
        msg = 'Trigger skips size too high'
        raise ValueError(msg)

    # Load instructions to device
    command = f':clock{clock_id}:trigger:data {trigger_skips},{trigger_delay},{trigger_reps}'
    resp = device_comm_write(
        device,
        command
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

    Notes
    -----
    This function MUST be called before loading clock data/instructions as
    it directly influences how the clock cycles are calculated. Every time
    clock resolution or clock data units are changed, reload the existing
    instructions.
    """
    clock_id = clock_params['clock_id']
    clock_res = clock_params['clock_res']
    freq_units = clock_params['reps_freq_units']
    trigger_id = clock_params['ext_trigger_id']
    clock_type = int(clock_params['ext_trigger'] == 'enabled')

    # Activate clock channel
    command = f':clock{clock_id}:state on'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp
        
    # Load clock divider
    command = f':clock{clock_id}:divider {clock_res}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock units
    command = f':clock{clock_id}:units {freq_units}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load external trigger pin
#    command = f':clock{clock_id}:enable'
#    resp = device_comm_write(
#        device,
#        command
#    )
#
#    for msg in resp:
#        if 'error' in msg.lower():
#            return resp

    # Load clock type
    if clock_type == 0: # Internal trigger
        mode = 'internal'
    elif clock_type == 1: # External trigger rising edge
        mode = 'rising'
    else:
        msg = 'Invalid clock type encountered'
        raise ValueError(msg)
        
    command = f':clock{clock_id}:trigger:mode {mode}'
    resp = device_comm_write(
        device,
        command
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

    # Convert delay instructions to cycles in ns
    output_inst, delay_inst = _convert_pulse_inst(pulse_params)

    # Validate that outputs and delays have equal length for variable timing
    output_len = len(output_inst)
    delay_len = len(delay_inst)
    
    if output_len != delay_len:
        msg = f'Reps and delay size mismatch: {output_len} vs {delay_len}'
        raise ValueError(msg)

    # Send information to clock data buffer
    output_inst_char = ",".join(map(str, output_inst))
    
    command = f':pulse:data:store:outputs {output_inst_char}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Send information to clock data buffer (for delays this time)
    delay_inst_char = ",".join(map(str, delay_inst))
    
    command = f':pulse:data:store:delays {delay_inst_char}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Now apply the current setting to the clock sequencer
    command = f':clock:data:store:apply {pulse_id}'
    resp = device_comm_write(
        device,
        command
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

    Notes
    -----
    This function MUST be called before loading pulse data/instructions as
    it directly influences how the clock cycles are calculated. Every time
    clock resolution or pulse data units are changed, reload the existing
    instructions.
    """
    pulse_id = pulse_params['pulse_id']
    pulse_units = pulse_params['pulse_units']
    trigger_id = pulse_params['clock_id']
    clock_res = pulse_params['clock_res']

   # Activate pulse channel
    command = f':pulse{pulse_id}:state  on'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp
        
    # Load pulse divider 
    command = f':pulse{pulse_id}:divider {clock_res}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse sequence units
    command = f':pulse{pulse_id}:units {pulse_units}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse signal pin
    command = f':pulse{pulse_id}:pin {trigger_id}'
    resp = device_comm_write(
        device,
        command
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

    command = f':clock{clock_id}:reset' 
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

    command = f':pulse{pulse_id}:reset' 
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
    # Reset all clock channels
    for channel_id in VALID_CLOCK_IDS:
        command = f':clock{channel_id}:reset' 
        resp = device_comm_write(device, command)

        for msg in resp:
            if 'error' in msg.lower():
                return resp

    # Reset all pulse channels
    for channel_id in VALID_CLOCK_IDS:
        command = f':pulse{channel_id}:reset' 
        resp = device_comm_write(device, command)
        for msg in resp:
            if 'error' in msg.lower():
                return resp

    return resp