__all__ = [
    'DeviceSystemError'
]


class DeviceSystemError(Exception):
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)