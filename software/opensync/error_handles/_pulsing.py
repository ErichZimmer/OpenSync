__all__ = [
    'PulseParamsSize'
]


class PulseParamsSize(Exception):
    """The error raised when the pulse parameters are invalid.

    As implemented, the pulse parameters container can only hold a certain
    number of instructions and must be in a specific structure. If an invalid
    pulse instruction is recognized, it is important to issue this error to
    notify the user of their error.
    
    """
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)
    