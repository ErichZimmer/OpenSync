__all__ = [
    'DeviceBufferSize',
    'DeviceBufferSizeWarning'
]


class DeviceBufferSize(Exception):
    """
    The error raised when the buffer size is exceeded.

    As implemented, the device buffer can only hold a certain number of
    instructions. Once exceeded, the OpenSync device will stop recieving
    instructions and issue a failed response.
    """
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)

    
class DeviceBufferSizeWarning(RuntimeWarning):
     """The warning raised when the buffer size is exceeded.

    As implemented, the device buffer can only hold a certain number of
    instructions. Once exceeded, the OpenSync device will stop recieving
    instructions and issue a failed response.

    """
    