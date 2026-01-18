from .._communication._io import device_comm_write
from .._error_handles import DeviceSystemError


__all__ = [
    'device_system_version',
    'device_system_freq',
    'device_system_status',
    'device_system_test',
    'device_system_fire',
    'device_system_stop'
]


def device_system_version(device: 'opensync') -> list[str]:
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
    command = '*IDN?'
    return device_comm_write(device, command)


def device_system_freq(device: 'opensync') -> list[str]:
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
    command = ':system:freq?'
    return device_comm_write(device, command)

    
def device_system_status(device: 'opensync') -> list[str]:
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
    command = ':system:status?'
    return device_comm_write(device, command)


def device_system_test(device: 'opensync') -> None:
    """Test opensync device for possible malfunctions.

    This function sends a few commands to the OpenSync device to make sure
    it is functioning as intended. If an error is encountered, an error is
    raised.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device to which the commands will be sent.

    Returns
    -------
    None
    """
    INVALID_RESPONSE = 'error'
    TEST_SUCCESS = '0'

    # Test for invalid commands
    command = 'some_invalid_command'
    resp = device_comm_write(device, command)

    # The first response string contains the string 'error'
    if INVALID_RESPONSE not in resp[0]:
        msg = 'OpenSync device recieved an unexpected response for invalid ' +\
            'command'
        raise DeviceSystemError(msg)

    # Device self-test
    command = '*TST?'
    resp = device_comm_write(device, command)

    # The first response will be '0' if all test passed.
    if resp[0] != TEST_SUCCESS:
        msg = 'OpenSync device failed device self-test'
        raise DeviceSystemError(msg)
        

def device_system_fire(device: 'opensync') -> list[str]:
    """Execute a pulse sequence from the OpenSync device buffer.

    This function sends a command to the OpenSync device to execute a fire
    operation for the specified number of repetitions.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device on which to execute the command.

    Returns
    -------
    response : list[str]
        A list of strings containing the response from the device after
        executing the command.
    """
    command = ':system:start'
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
    command = ':system:stop'
    return device_comm_write(device, command)