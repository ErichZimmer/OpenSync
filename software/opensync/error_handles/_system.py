__all__ = [
    'DeviceSystemError'
]


class DeviceSystemError(Exception):
    """
    The error raised with invalid device configurations.

    As implemented, the OpenSync device requires a certain configuration to
    operate. Configurations beyond the defined oeprating configurations may
    lead to undefined behavior.
    """
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)