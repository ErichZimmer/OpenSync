from serial import Serial, SerialException
from serial.tools import list_ports
from contextlib import contextmanager


BAUDRATE = 19200
READ_TIMEOUT = 0.1
WRITE_TIMEOUT = 0.1
READ_TIMEOUT_FAST = 0.02
WRITE_TIMEOUT_FAST = 0.01
EOL = '\r\n'
DELIMITER = ','


__all__ = [
    'EOL',
    'device_comm_search',
    'device_comm_open',
    'device_comm_write',
    'device_comm_close',
    'device_comm_managed'
]


def _get_response(device: 'opensync') -> str:
    response = device.readlines()
    return response
    

def _parse_response(response: str) -> list[str]:
    # Remove terminator
    response = [resp.decode().replace(EOL, '') for resp in response]

    # Remove commas
    response = [resp.split(DELIMITER) for resp in response]

    # Flatten the list of response(s)
    response = [resp for inner in response for resp in inner]

    return response


def _check_if_opensync(device: 'device') -> bool:
    EXPECTED = 'opensync'
    
    resp = device_comm_write(
        device,
        '*IDN?'
    )

    # Check if 'opensync' is in the response(s)
    for response in resp:
        if EXPECTED in response.lower():
            return True
            
    return False
    

def device_comm_search() -> list[str]:
    """Search for available serial ports.

    This function scans the system for available serial ports and returns
    a list of devices that are currently connected.

    Returns
    -------
    ports : list[str]
        A list of strings representing the names of the connected serial
        ports.

    Example
    -------
    >>> opensync_port = device_comm_search()
    >>> print(opensync_port)
    ['COM3']
    """
    comlist = list_ports.comports()

    # Get all com active ports
    connected = []
    for port in comlist:
        connected.append(port.device)

    # Probe each one to find an OpenSync Device
    devices = []
    for port in connected:
        try:
            with device_comm_managed(port) as device:
                # Probe port for an acceptable response
                if _check_if_opensync(device):
                    devices.append(port)
                
        except SerialException:
            pass

    return devices


def device_comm_open(
    port: str,
    fast: bool=False
) -> 'opensync':
    """Open a connection to a specified serial port.

    This function initializes a new OpenSync device connection on the
    specified serial port with predefined baud rate and timeouts.

    Parameters
    ----------
    port : str
        The name of the serial port to which the OpenSync device will
        connect.
    fast : bool
        If enabled, decrease the serial response wait time by over half.

    Returns:
    -------
    device : 'opensync'
        An instance of the OpenSync device that is connected to the
        specified port.

    Raises
    ------
    SerialException
        If the connection to the specified port cannot be established.
    """
    if fast:
        read_timeout = READ_TIMEOUT_FAST
        write_timeout = WRITE_TIMEOUT_FAST
    else:
        read_timeout = READ_TIMEOUT
        write_timeout = WRITE_TIMEOUT
        
    device = Serial()
    device.baudrate = BAUDRATE
    device.port = port
    device.timeout = read_timeout
    device.write_timeout = write_timeout
    
    device.open()

    return device


def device_comm_write(
    device: 'opensync',
    command: str, 
    output: bool=True,
    eol: str=EOL
) -> list[str]:
    """Send a command to the OpenSync device.

    This function writes a command to the OpenSync device, appending an
    end-of-line character, and retrieves the response from the device.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device to which the command will be sent.
    command : str
        The command string to be sent to the device.
    output : bool
        Return the retrieved device output if expecting one.
    eol : str, optional
        The end-of-line character(s) to append to the command
        (default is '\\r\\n').

    Returns
    -------
    response : list[str]
        A list of strings containing the responses from the OpenSync device
        after executing the command.
    """
    command += eol

    # Send command terminated in CRLF or any other terminator
    device.write(command.encode())
    
    if output:
        return _parse_response(_get_response(device))

    
def device_comm_close(device: 'opensync') -> None:
    """Close the connection to the OpenSync device.

    This function closes the serial connection to the OpenSync device,
    releasing any resources associated with the connection.

    Parameters
    ----------
    device : 'opensync'
        An instance of the OpenSync device that will be closed.
        
    Returns
    -------
    None
    """
    device.close()


@contextmanager
def device_comm_managed(
    port: str,
    fast: bool=False
) -> None:
    """Context manager for managing the OpenSync device connection.

    This context manager opens a connection to the specified serial port
    and ensures that the connection is properly closed when the context
    is exited.

    Parameters
    ----------
    port : str
        The name of the serial port to which the OpenSync device will
        connect.
    fast : bool
        If enabled, decrease the serial response wait time by over half.

    Yields
    -------
    device : 'opensync'
        An instance of the OpenSync device that is connected to the
        specified port.
    """
    device = device_comm_open(
        port,
        fast
    )
    
    try:
        yield device
    finally:
        device_comm_close(device)