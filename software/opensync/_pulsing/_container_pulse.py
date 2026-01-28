from typing import Tuple
from .._error_handles import PulseParamsError
from ._utils import _get_channel_ids
from ._container_clock import VALID_CLOCK_IDS, VALID_CLOCK_DIVIDERS

VALID_PULSE_UNITS = [
    'nano', 'nanosecond',
    'micro', 'microsecond',
    'milli', 'millisecond',
    'sec', 'second',
    'min', 'minute',
    'hour'
]
MIN_PULSE_TRAIN_SIZE = 1
PULSE_SEQUENCE_SIZE = 2


__all__ = [
    'get_pulse_params',
    'config_pulse_id',
    'config_pulse_clock_id',
    'config_pulse_res',
    'config_pulse_units',
    'insert_pulse',
    'insert_pulse_many'
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
        - 'pulse_id': int 
            An integer between 0 and 2 representing the pulse
            channel to use (default is 0).
        - 'clock_id' : int
            An integer between 0 and 2 representing the clock 
            channel to use (default is 0).
        - 'clock_res' : str
            A string representing the clock divider (default is high_res).
        - 'pulse_units' : str
            A string representing the pulse data units..
        - 'channel_X' : list[float]
            A dict of channel name and pulse data for each channel where X 
            is the channel number from 0 to 11.
    
    """
    pulse_params = {
        'pulse_id': 0,
        'clock_id': 0,
        'clock_res': 'high_res',
        'pulse_units': 'microsecond',
        'channel_0': [],
        'channel_1': [],
        'channel_2': [],
        'channel_3': [],
        'channel_4': [],
        'channel_5': [],
        'channel_6': [],
        'channel_7': [],
    }

    channels = _get_channel_ids(pulse_params)

    for channel in channels:
        pulse_params[channel] = _get_empty_container(name=channel)
        
    return pulse_params


def config_pulse_id(
    pulse_params: dict,
    channel_id: int=0
) -> dict:
    """Configure which pulse channel to use.

    This function updates the pulse pararameters by selecting which clock
    channel to use for the internal timing and execution of the pulse
    parameters.

    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    channel_id : int
        The index of the pulse channel to which the pulse parameters will be
        executed. Valid pulse ids are 0, 1, and 2.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.
    
    """
    if channel_id not in VALID_CLOCK_IDS:
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    pulse_params['pulse_id'] = channel_id

    return pulse_params


def config_pulse_clock_id(
    pulse_params: dict,
    channel_id: int=0
) -> dict:
    """Configure which clock channel to use for pulsing.

    This function updates the pulse pararameters by selecting which clock
    channel to use for the internal timing and execution of the pulse
    parameters.

    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    channel_id : int
        The index of the clock channel to which the pulse parameters will be
        controlled. Valid clock ids are 0, 1, and 2.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.
    
    """
    if channel_id not in VALID_CLOCK_IDS:
        msg = f'Invalid clock channel selected. Got {channel_id}'
        raise ValueError(msg)

    pulse_params['clock_id'] = channel_id

    return pulse_params
    

def config_pulse_res(
    pulse_params: dict,
    clock_res: str='high'
) -> dict:
    """Configure the clock resolution for a pulse output channel.

    This function updates the pulse pararameters by modfying the clock
    divider of that particular channel. This effectively slows the clock down
    by dividing the main clock by the clock divider.

    Params
    ------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    clock_res : str
        The clock divider resolution. The following are accepted values:

        'high', 'high_res'
            The clock divider is set to 1 effectively allowing for a
            clock cycle resolution of 4 nanosecondsns.

        'med', 'med_res'
            The clock divider is set to 2 effectively allowing for a
            clock cycle resolution of 8 nanoseconds.

        'low', 'low_res'
            The clock divider is set to 5 effectively allowing for a
            clock cycle resolution of 20 nanoseconds.

        'very_low', 'very_low_res'
            The clock divider is set to 25 effectively allowing for a
            clock cycle resolution of 100 nanoseconds.

        'very_very_low', 'very_very_low_res'
            The clock divider is set to 250 effectively allowing for a
            clock cycle resolution of 1 microsecond.

    Returns
    -------
    pulse_params : dict
        The updated clock parameters dictionary.
    
    """
    if clock_res.lower() not in VALID_CLOCK_DIVIDERS:
        msg = f'Invalid clock divider resolution. Got {clock_res}'
        raise ValueError(msg)

    pulse_params['clock_res'] = clock_res

    return pulse_params


def config_pulse_units(
    pulse_params: dict,
    units: str='microsecond'
) -> dict:
    """Configure the units for a pulse output channel.

    This function updates the pulse pararameters by modfying the units of
    the pulse sequencer. This makes it easier to produce either very short
    or long pulses by simply changing the data units.

    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    units : str
        The pulse data units. The following are accepted values:

        'nano', 'nanosecond'
            The pulse data is in nanoseconds.

        'micro', 'microsecond'
            the pulse data is in microseconds.

        'milli', 'millisecond'
            the pulse data is in milliseconds.

        'sec', 'seconds'
            the pulse data is in seconds.

        'min', 'minute'
            the pulse data is in minutes.

        'hour'
            the pulse data is in hours.

    Returns
    -------
    pulse_params : dict
        The updated clock parameters dictionary.
    
    """
    if units.lower() not in VALID_PULSE_UNITS:
        msg = f'Invalid data units. Got {units}'
        raise ValueError(msg)

    pulse_params['pulse_units'] = units

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
    """Insert simple pulse timing information into the specified channel.

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

    Notes
    -----
    If either rising_edge_2 or falling_edge_2 is missing, then the entire
    second pulse sequence is omitted.
    
    """
    channel = _get_channel_ids(pulse_params)[channel_id]

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


def insert_pulse_many(
    pulse_params: dict,
    pulse_train: list[Tuple[float, float]],
    channel_id: int = 0,
    channel_name=None
) -> dict:
    """Insert several pulse timing information into the specified channel.

    This function updates the pulse parameters dictionary by inserting the
    start and end times of many pulses into the specified channel. If a
    channel name is provided, it updates the channel's name as well.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    pulse_train : list[tuple[float, float]]
        The start and end times of each pulse in microseconds.
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

    Raises
    ------
    PulseParamsSize
        If invalid pulse train instructions are detected, this error is
        raised.
    
    """
    channel = _get_channel_ids(pulse_params)[channel_id]
    pulse_data = []
    
    pulse_train_size = len(pulse_train)
    if pulse_train_size < MIN_PULSE_TRAIN_SIZE:
        msg = f'Invalid pulse train size of {pulse_train_size}'
        raise PulseParamsError(msg)

    # Check for pulse train size and append
    for sequence in pulse_train:
        sequence_size = len(sequence)
        if sequence_size != PULSE_SEQUENCE_SIZE:
            msg = f'Invalid pulse sequence size of {sequence_size} detected'
            raise PulseParamsSize(msg)

        # If we made it this far, the evrything should be a-okay
        pulse_data += sequence

    pulse_params[channel]['data'] = pulse_data

    if channel_name != None:
        pulse_params[channel]['name'] = channel_name

    return pulse_params