__all__ = [
    'DeviceBufferSizeError',
]


class DeviceBufferSizeError(Exception):
    """The error raised when the buffer size is exceeded.

    As implemented, the device buffer can only hold a certain number of
    instructions. Once exceeded, the OpenSync device will stop recieving
    instructions and issue a failed response.
    
    """
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)