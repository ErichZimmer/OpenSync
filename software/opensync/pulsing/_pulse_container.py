from ._utils import get_channel_ids


__all__ = [
    'get_pulse_params',
    'insert_pulse'
]


def _get_empty_container(name: str='') -> dict:
    container = {
        'name': name,
        'data': []
    }

    return container


def get_pulse_params() -> dict:
    """Retrieve the default pulse parameters for the pulse generator.

    This function initializes and returns a dictionary containing the
    default pulse parameters for a pulse generator. The dictionary includes
    settings such as the  empty lists for each channel's pulse data.

    Returns
    -------
    pulse_params : dict
        A dictionary containing the default pulse parameters. The structure
        of the dictionary includes:
        - 'channel_X': A dict of channel name and pulse data for each channel
           (where X is the channel number from 0 to 11).
    """
    pulse_params = {
        'channel_0': [],
        'channel_1': [],
        'channel_2': [],
        'channel_3': [],
        'channel_4': [],
        'channel_5': [],
        'channel_6': [],
        'channel_7': [],
        'channel_8': [],
        'channel_9': [],
        'channel_10': [],
        'channel_11': [],
    }

    channels = get_channel_ids(pulse_params)

    for channel in channels:
        pulse_params[channel] = _get_empty_container(name=channel)
        
    return pulse_params

    
def insert_pulse(
    pulse_params: dict,
    rising_edge_1: int,
    falling_edge_1: int,
    rising_edge_2: int = None,
    falling_edge_2: int = None,
    channel_id: int = 0,
    channel_name=None
) -> dict:
    """Insert pulse timing information into the specified channel.

    This function updates the pulse parameters dictionary by inserting the
    start and end times of one or two pulses into the specified channel. If
    a channel name is provided, it updates the channel's name as well.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    rising_edge_1 : int
        The start time of the first pulse in microseconds.
    falling_edge_1 : int
        The end time of the first pulse in microseconds.
    rising_edge_2 : int, optional
        The start time of the second pulse in microseconds. If not provided,
        no second pulse is added.
    falling_edge_2 : int, optional
        The end time of the second pulse in microseconds. Must be provided
        if rising_edge_2 is provided.
    channel_id : int, optional
        The index of the channel to which the pulses will be added.
    channel_name : str, optional
        The name to assign to the channel. If provided, it will update the
        channel's name.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary with the inserted pulse
        timing information.
    """
    channel = get_channel_ids(pulse_params)[channel_id]

    pulse_train = [
        rising_edge_1,
        falling_edge_1
    ]

    if (rising_edge_2 != None) and (falling_edge_2 != None):
        pulse_train += [
            rising_edge_2,
            falling_edge_2
        ]

    pulse_params[channel]['data'] = pulse_train

    if channel_name != None:
        pulse_params[channel]['name'] = channel_name

    return pulse_params