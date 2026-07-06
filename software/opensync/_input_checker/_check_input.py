__all__ = [
    'check_types'
]


def check_types(
    input_type,
    **kwargs
):
    for arg in kwargs:
        param = kwargs[arg]

        if not isinstance(param, input_type):
            msg = f'{arg} ({type(param)}) is not of type {input_type}'
            raise TypeError(msg)
            