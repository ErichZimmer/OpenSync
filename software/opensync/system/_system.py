from ..communication._io import device_comm_write
from ..error_handles import DeviceSystemError


__all__ = [
    'device_get_version',
    'device_get_freq',
    'device_get_status',
    'device_system_test',
    'device_system_fire',
    'device_system_stop'
]


def device_get_version(device: 'opensync') -> list[str]:
    """Retrieve the version information from the OpenSync device.

    This function sends a command to the OpenSync device to obtain its
    version information.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device from which to retrieve the
        version.

    Returns
    -------
    response : list[str]
        A list of strings containing the version information of the device.
    """
    command = 'vers'
    return device_comm_write(device, command)


def device_get_freq(device: 'opensync') -> list[str]:
    """Retrieve the frequency information from the OpenSync device.

    This function sends a command to the OpenSync device to obtain its
    clock frequency information.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device from which to retrieve the
        frequency.

    Returns
    -------
    response : list[str]
        A list of strings containing the frequency information of the 
        device.
    """
    command = 'freq'
    return device_comm_write(device, command)

    
def device_get_status(device: 'opensync') -> list[str]:
    """Retrieve the current status from the OpenSync device.

    This function sends a command to the OpenSync device to obtain its
    current operational status.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device from which to retrieve the status.

    Returns
    -------
    response : list[str]
        A list of strings containing the current status of the device.
    """
    command = 'stat'
    return device_comm_write(device, command)


def device_system_test(device: 'opensync') -> None:
    """Test opensync device status and clock frequencies.

    This function sends a few commands to the OpenSync device to make sure
    it is functioning as intended.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device to which the commands will be sent.

    Returns
    -------
    None
    """
    EXPECTED_CLOCK_MAIN_FREQ = 400000 # KHz
    CLOCK_MAIN_INDEX = 3
    VALID_RESPONSE = 'ok'
    INVALID_RESPONSE = 'Invalid Response'

    if resp[-1] != VALID_RESPONSE:
        raise DeviceSystemError(
            'OpenSync device recieved an unexpected response for valid command'
        )

    # Test for invalid commands
    command = 'some_invalid_command'
    resp = device_comm_write(device, command)

    if INVALID_RESPONSE not in resp[-1]:
        raise DeviceSystemError(
            'OpenSync device recieved an unexpected response for invalid ' + 
            'command'
        )
        
    # Test main clock frequency
    resp = device_get_freq(device)
    main_clock_freq = resp[CLOCK_MAIN_INDEX]

    if main_clock_freq != EXPECTED_CLOCK_MAIN_FREQ:
        raise DeviceSystemError(
            'OpenSync device is operating at an unsupport clock frequency'
        )

    # Test for valid commands
    resp = device_get_version(device)
    

def device_system_fire(
    device: 'opensync',
    reps: int
) -> list[str]:
    """Execute a pulse sequence from the OpenSync device buffer.

    This function sends a command to the OpenSync device to execute a fire
    operation for the specified number of repetitions.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device on which to execute the command.
    reps : int
        The number of repetitions for the fire command.

    Returns
    -------
    response : list[str]
        A list of strings containing the response from the device after
        executing the command.
    """
    command = 'fire ' + str(reps)
    return device_comm_write(device, command)


def device_system_stop(device: 'opensync') -> list[str]:
    """Stop the current operation of the OpenSync device.

    This function sends a command to the OpenSync device to stop its current
    operation.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device on which to execute the stop
        command.

    Returns
    -------
    response : list[str]
        A list of strings containing the response from the device after
        executing the stop command.
    """
    command = 'stop'
    return device_comm_write(device, command)