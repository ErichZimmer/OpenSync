from .._communication._io import device_comm_write
from ._conversion import _convert_pulse_inst
from ._container_clock import MAX_SKIPS, VALID_CLOCK_IDS


__all__ = [
    '_device_clock_config_load',
    '_device_pulse_config_load',
    'device_clock_reset',
    'device_pulse_reset',
    'device_timing_reset'
]

    
def _device_clock_config_load(
    device: 'opensync',
    clock_params: 'clock_params'
) -> list[str]:
    """Load clock config into an OpenSync device.

    Load a clock config into the chosen clock channel of an OpenSync device.
    All information is uploaded, even those that have not been modified.
    
    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.

    Returns
    -------
    response : list[str]
        A list of strings with error messages if present, or an empty list
        if no errors are encountered.

    Notes
    -----
    - If an error occurs during the command execution, the command will exit
      prematurely and return the recieved error.
    
    """
    clock_id      = clock_params['clock_id']
    clock_res     = clock_params['clock_res']
    clock_mode    = clock_params['clock_mode']
    clock_units   = clock_params['clock_units']
    clock_freq    = clock_params['clock_freq']
    clock_iter    = clock_params['clock_iter']
    trigger_id    = clock_params['trigger_id']
    trigger_mode  = clock_params['trigger_mode']
    trigger_edge  = clock_params['trigger_edge']
    trigger_level = clock_params['trigger_level']
    trigger_units = clock_params['trigger_units']
    trigger_delay = clock_params['trigger_delay']
    trigger_skips = clock_params['trigger_skips']
    trigger_count = clock_params['trigger_count']  
       
    # Load clock divider
    command = f':source:clock{clock_id}:divider {clock_res}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock units
    command = f':source:clock{clock_id}:units {clock_units}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load clock mode
    command = f':source:clock{clock_id}:mode {clock_mode}'
    resp = device_comm_write(
        device,
        command
    )
    
    for msg in resp:
        if 'error' in msg.lower():
            return resp

   # Validate that reps and frequencies have equal length for variable timing
    reps_len = len(clock_iter)
    freq_len = len(clock_freq)
    
    if reps_len != freq_len:
        msg = f'Reps and frequency size mismatch: {reps_len} vs {freq_len}'
        raise ValueError(msg)

    # Send information to clock data buffer
    if reps_len > 0:
        reps_inst_char = ",".join(map(str, clock_iter))
    
        command = f':source:clock{clock_id}:data:buffer:count {reps_inst_char}'
        resp = device_comm_write(
            device,
            command
        )
    
        for msg in resp:
            if 'error' in msg.lower():
                return resp
            
    # Send information to clock data buffer (for frequencies this time)
    if freq_len > 0:
        freq_inst_char = ",".join(map(str, clock_freq))
        
        command = f':source:clock{clock_id}:data:buffer:frequency {freq_inst_char}'
        resp = device_comm_write(
            device,
            command
        )
    
        for msg in resp:
            if 'error' in msg.lower():
                return resp
    
    # Now apply the current setting to the clock sequencer
    if (reps_len > 0) and (freq_len > 0):
        command = f':source:clock{clock_id}:data:buffer:apply'
        resp = device_comm_write(
            device,
            command
        )

        for msg in resp:
            if 'error' in msg.lower():
                return resp
    
    # Load trigger id
    command = f':trigger:clock{clock_id}:input {trigger_id}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger units
    command = f':trigger:clock{clock_id}:units {trigger_units}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger mode
    command = f':trigger:clock{clock_id}:mode {trigger_mode}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger edge
    command = f':trigger:clock{clock_id}:edge {trigger_edge}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger gate level
    command = f':trigger:clock{clock_id}:gate:level {trigger_level}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger delay
    command = f':trigger:clock{clock_id}:delay {trigger_delay}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger skips
    command = f':trigger:clock{clock_id}:skip {trigger_skips}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load trigger count
    command = f':trigger:clock{clock_id}:count {trigger_count}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp
    
    # Activate clock channel if everything is okay
    command = f':source:clock{clock_id}:state on'
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
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the load command. Each string corresponds to a 
        response from the device.

    Notes
    -----
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

    # Send information to pulse data buffer
    output_inst_char = ",".join(map(str, output_inst))
    
    command = f':source:pulse{pulse_id}:data:buffer:output {output_inst_char}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Send information to pulse data buffer (for delays this time)
    delay_inst_char = ",".join(map(str, delay_inst))
    
    command = f':source:pulse{pulse_id}:data:buffer:delay {delay_inst_char}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Now apply the current setting to the pulse sequencer
    command = f':source:pulse{pulse_id}:data:buffer:apply'
    resp = device_comm_write(
        device,
        command
    ) 

    return resp


def _device_pulse_config_load(
    device: 'opensync',
    pulse_params: 'pulse_params'
) -> list[str]:
    """Load pulse configs into a chosen pulse channel of an OpenSync device.

    Load a pulse config into the chosen pulse channel of an OpenSync device.
    All information is uploaded, even those that have not been modified.
    
    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will receive the commands.
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    response : list[str]

    Notes
    -----
    - If an error occurs during the command execution, the command will exit
      prematurely and return the recieved error.
    
    """
    pulse_id    = pulse_params['pulse_id']
    pulse_units = pulse_params['pulse_units']
    trigger_id  = pulse_params['clock_id']
    clock_res   = pulse_params['clock_res']
        
    # Load pulse divider 
    command = f':source:pulse{pulse_id}:divider {clock_res}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse sequence units
    command = f':source:pulse{pulse_id}:units {pulse_units}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse signal pin
    command = f':source:pulse{pulse_id}:input {trigger_id}'
    resp = device_comm_write(
        device,
        command
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Load pulse instructions
    resp = _device_pulse_inst_load(
        device,
        pulse_params
    )

    for msg in resp:
        if 'error' in msg.lower():
            return resp

    # Activate pulse channel if everything is okay to this point
    command = f':source:pulse{pulse_id}:state on'
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