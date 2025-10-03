from serial import Serial
from serial.tools import list_ports
from contextlib import contextmanager


BAUDRATE = 19200
READ_TIMEOUT = 0.25
WRITE_TIMEOUT = 0.25
CLOCK_CYCLE = 4.0 # clock cycle in nanoseconds
EOL = '\r\n'


__all__ = [
    'CLOCK_CYCLE',
    'EOL',
    'device_comm_search',
    'device_comm_open',
    'device_comm_write',
    'device_comm_close',
    'device_comm_managed'
]


def _get_response(device: 'opensync') -> str:
    return device.readlines()
    

def _parse_response(response: str) -> list[str]:
    return [line.decode().replace(EOL, '') for line in response]

    
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
    >>> connected_ports = device_comm_search()
    >>> print(connected_ports)
    ['COM3', 'COM4']
    """
    comlist = list_ports.comports()

    connected = []
    for port in comlist:
        connected.append(port.device)

    return connected


def device_comm_open(port: str) -> 'opensync':
    """Open a connection to a specified serial port.

    This function initializes a new OpenSync device connection on the
    specified serial port with predefined baud rate and timeouts.

    Parameters
    ----------
    port : str
        The name of the serial port to which the OpenSync device will
        connect.

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
    device = Serial()
    device.baudrate = BAUDRATE
    device.port = port
    device.timeout = READ_TIMEOUT
    device.write_timeout = WRITE_TIMEOUT
    
    device.open()

    return device


def device_comm_write(
    device: 'opensync',
    command: str, 
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
def device_comm_managed(port: str) -> None:
    """Context manager for managing the OpenSync device connection.

    This context manager opens a connection to the specified serial port
    and ensures that the connection is properly closed when the context
    is exited.

    Parameters
    ----------
    port : str
        The name of the serial port to which the OpenSync device will
        connect.

    Yields
    -------
    device : 'opensync'
        An instance of the OpenSync device that is connected to the
        specified port.
    """
    device = device_comm_open(port)
    
    try:
        yield device
    finally:
        device_comm_close(device)