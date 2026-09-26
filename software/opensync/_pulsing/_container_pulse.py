from typing import Tuple

from .._input_checker import check_types
from .._error_handles import PulseParamsError


VALID_SYNC_SOURCES = [
    't0',
    'cha',
    'chb',
    'chc',
    'chd',
    'che',
    'chf',
    'chg',
    'chh'
]
VALID_OUTPUT_LEVELS = [
    'ttl',
    'lvttl'
]
COUNTERS_MAX = 1000000000
BCOUNTER_MAX = 30000000
DIVIDER_MAX = 65500
MIN_PULSE_TRAIN_SIZE = 1
MAX_PULSE_TRAIN_SIZE = 2
PULSE_SEQUENCE_SIZE = 2
MIN_PULSE_LENGTH = 44e-9
MAX_PULSE_LENGTH = 8.0
DECIMALS_NS = 9


__all__ = [
    'get_pulse_params',
    'config_pulse_state',
    'config_pulse_divider',
    'config_pulse_insert',
    'config_pulse_wcounter',
    'config_pulse_bcounter',
    'config_pulse_pcounter',
    'config_pulse_ocounter',
    'config_pulse_sync',
    'config_pulse_output_level'
]


def _get_empty_container(name: str = '') -> dict:
    container = {
        'name': name,
        'state': False,
        'divider': 1,
        'data': [],
        'wcounter': 0,
        'bcounter': 0,
        'pcounter': 1,
        'ocounter': 0,
        'sync': 't0',
        'output_level': 'lvttl'
    }

    return container


def get_pulse_params() -> dict:
    """Retrieve the default pulse parameters for the pulse generator.

    This function initializes and returns a dictionary containing the
    configuration and pulse data for each output channel.

    Returns
    -------
    pulse_params : dict
        A dictionary containing channels 'channel_0' through 'channel_7'.
        Each channel contains:
        - 'name' : str
            The channel name.
        - 'state' : bool
            Whether the channel is enabled (default is False).
        - 'divider' : int
            The channel clock divider (default is 1).
        - 'data' : list[float]
            Flattened rising/falling edge times in seconds.
        - 'wcounter' : int
            The initial synchronization events to skip (default is 0).
        - 'bcounter' : int
            The total waveform buffers to execute (default is 0).
        - 'pcounter' : int
            The ON synchronization event count (default is 1).
        - 'ocounter' : int
            The OFF synchronization event count (default is 0).
        - 'sync' : str
            The synchronization source (default is 't0').
        - 'output_level' : str
            The output logic level (default is 'lvttl').

    Notes
    -----
    - Python channel indices 0 through 7 correspond to firmware output
      channels 1 through 8, or CHA through CHH.

    """
    pulse_params = {
        'channel_1': [],
        'channel_2': [],
        'channel_3': [],
        'channel_4': [],
        'channel_5': [],
        'channel_6': [],
        'channel_7': [],
        'channel_8': []
    }

    for channel in pulse_params:
        pulse_params[channel] = _get_empty_container(name=channel)

    return pulse_params


def config_pulse_state(
    pulse_params: dict,
    state: bool,
    channel_id: int = 0
) -> dict:
    """Configure the pulse channel state.

    This function updates the pulse parameters to enable or disable the
    selected output channel.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    state : bool
        True enables the channel. False disables it.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'state' key.

    """
    check_types(
        bool,
        state=state
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]
    pulse_params[channel]['state'] = state

    return pulse_params


def config_pulse_divider(
    pulse_params: dict,
    divider: int = 1,
    channel_id: int = 0
) -> dict:
    """Configure the pulse channel clock divider.

    This function updates the pulse parameters to set the clock divider
    of the selected output channel.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    divider : int
        The desired clock divider, from 1 through 65500.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'divider' key.

    """
    check_types(
        int,
        divider=divider
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if divider < 1 or divider > DIVIDER_MAX:
        msg = f'Invalid pulse divider. Got {divider}'
        raise ValueError(msg)

    pulse_params[channel]['divider'] = divider

    return pulse_params


def config_pulse_insert(
    pulse_params: dict,
    pulse_train: list[Tuple[float, float]],
    channel_id: int = 0,
    channel_name=None,
    units: str = 's'
) -> dict:
    """Insert pulse timing information into the specified channel.

    This function updates the pulse parameters dictionary by inserting the
    start and end times of one or two pulses into the specified channel.
    Edge times are converted to seconds and rounded to the nearest
    nanosecond before being validated and stored.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    pulse_train : list[tuple[float, float]]
        One or two rising/falling edge pairs in the selected units.
        Edge times are measured from the start of the sequence.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.
    channel_name : str, optional
        The name to assign to the channel.
    units : str, optional
        The pulse timing units: 'ns', 'us', 'ms', or 's'.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Raises
    ------
    PulseParamsError
        If the pulse train size, edge pairs, widths, or delays are invalid.

    Notes
    -----
    - Each rounded pulse width must be between 44 nanoseconds and
      8 seconds, inclusive.
    - The initial delay and gaps between pulses must satisfy the same
      limits. The first pulse may start at zero without an initial delay.
    - Pulses must be supplied in chronological order.
    - The function replaces the selected channel's existing pulse data.
    - The 'data' key stores flattened rising/falling edge times in seconds.

    """
    check_types(
        list,
        pulse_train=pulse_train
    )

    check_types(
        int,
        channel_id=channel_id
    )

    check_types(
        (str, type(None)),
        channel_name=channel_name
    )

    check_types(
        str,
        units=units
    )

    unit_factors = {
        'ns': 1e-9,
        'us': 1e-6,
        'ms': 1e-3,
        's': 1.0
    }

    if units.lower() not in unit_factors:
        msg = f'Invalid pulse units. Got {units}'
        raise ValueError(msg)

    unit_scale = unit_factors[units.lower()]
    pulse_train_size = len(pulse_train)

    if not MIN_PULSE_TRAIN_SIZE <= pulse_train_size <= MAX_PULSE_TRAIN_SIZE:
        msg = f'Invalid pulse train size of {pulse_train_size}'
        raise PulseParamsError(msg)

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]
    pulse_data = []
    previous_falling_edge = 0.0

    for sequence in pulse_train:
        check_types(
            (tuple, list),
            sequence=sequence
        )

        sequence_size = len(sequence)

        if sequence_size != PULSE_SEQUENCE_SIZE:
            msg = f'Invalid pulse sequence size of {sequence_size} detected'
            raise PulseParamsError(msg)

        check_types(
            (float, int),
            rising_edge=sequence[0],
            falling_edge=sequence[1]
        )

        rising_edge = round(
            sequence[0] * unit_scale, 
            DECIMALS_NS
        )
        
        falling_edge = round(
            sequence[1] * unit_scale, 
            DECIMALS_NS
        )

        pulse_length = round(
            falling_edge - rising_edge, 
            DECIMALS_NS
        )

        if not MIN_PULSE_LENGTH <= pulse_length <= MAX_PULSE_LENGTH:
            msg = f'Invalid pulse length in seconds. Got {pulse_length}'
            raise PulseParamsError(msg)

        delay = round(
            rising_edge - previous_falling_edge,
            DECIMALS_NS
        )

        if pulse_data or delay != 0:
            if not MIN_PULSE_LENGTH <= delay <= MAX_PULSE_LENGTH:
                msg = f'Invalid pulse delay in seconds. Got {delay}'
                raise PulseParamsError(msg)

        pulse_data += [
            rising_edge,
            falling_edge
        ]

        previous_falling_edge = falling_edge

    pulse_params[channel]['data'] = pulse_data

    if channel_name != None:
        pulse_params[channel]['name'] = channel_name

    return pulse_params


def config_pulse_wcounter(
    pulse_params: dict,
    count: int,
    channel_id: int = 0
) -> dict:
    """Configure the initial wait counter.

    This function updates the pulse parameters to set the number of
    synchronization events skipped before the channel starts its P/O cycle.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    count : int
        The initial number of events to skip, from 0 through 1000000000.
        A value of 0 starts on the first accepted event.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'wcounter' key.
    - The initial wait is performed once per program run.

    """
    check_types(
        int,
        count=count
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if count < 0 or count > COUNTERS_MAX:
        msg = f'Invalid pulse W counter. Got {count}'
        raise ValueError(msg)

    pulse_params[channel]['wcounter'] = count

    return pulse_params


def config_pulse_bcounter(
    pulse_params: dict,
    count: int,
    channel_id: int = 0
) -> dict:
    """Configure the waveform buffer counter.

    This function updates the pulse parameters to set the total number
    of waveform buffers the channel will execute.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    count : int
        The number of waveform buffers to execute, from 0 through 30000000.
        A value of 0 selects infinite operation.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'bcounter' key.

    """
    check_types(
        int,
        count=count
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if count < 0 or count > BCOUNTER_MAX:
        msg = f'Invalid pulse B counter. Got {count}'
        raise ValueError(msg)

    pulse_params[channel]['bcounter'] = count

    return pulse_params


def config_pulse_pcounter(
    pulse_params: dict,
    count: int,
    channel_id: int = 0
) -> dict:
    """Configure the ON duty cycle counter.

    This function updates the pulse parameters to set the number of
    synchronization events that execute the channel's waveform buffer
    before the OFF portion of the P/O cycle.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    count : int
        The ON synchronization event count, from 0 through 1000000000.
        The firmware treats a value of 0 as 1.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'pcounter' key.

    """
    check_types(
        int,
        count=count
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if count < 0 or count > COUNTERS_MAX:
        msg = f'Invalid pulse P counter. Got {count}'
        raise ValueError(msg)

    pulse_params[channel]['pcounter'] = count

    return pulse_params


def config_pulse_ocounter(
    pulse_params: dict,
    count: int,
    channel_id: int = 0
) -> dict:
    """Configure the OFF duty cycle counter.

    This function updates the pulse parameters to set the number of
    synchronization events skipped after the ON portion of the P/O cycle.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    count : int
        The OFF synchronization event count, from 0 through 1000000000.
        A value of 0 means no OFF events are skipped.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'ocounter' key.

    """
    check_types(
        int,
        count=count
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if count < 0 or count > COUNTERS_MAX:
        msg = f'Invalid pulse O counter. Got {count}'
        raise ValueError(msg)

    pulse_params[channel]['ocounter'] = count

    return pulse_params


def config_pulse_sync(
    pulse_params: dict,
    sync: str,
    channel_id: int = 0
) -> dict:
    """Configure the pulse synchronization source.

    This function updates the pulse parameters to select the source whose
    rising events control the output channel.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    sync : str
        The synchronization source. Accepted values are 't0', 'cha', 'chb',
        'chc', 'chd', 'che', 'chf', 'chg', and 'chh'.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'sync' key.
    - A channel cannot select itself as its synchronization source.

    """
    check_types(
        str,
        sync=sync
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if sync.lower() not in VALID_SYNC_SOURCES:
        msg = f'Invalid pulse synchronization source. Got {sync}'
        raise ValueError(msg)

    if sync.lower() == VALID_SYNC_SOURCES[channel_id + 1]:
        msg = 'A pulse channel cannot synchronize to itself'
        raise ValueError(msg)

    pulse_params[channel]['sync'] = sync

    return pulse_params


def config_pulse_output_level(
    pulse_params: dict,
    level: str,
    channel_id: int = 0
) -> dict:
    """Configure the pulse output logic level.

    This function updates the pulse parameters to select the output logic
    level of the channel.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    level : str
        The selected output logic level: 'ttl' or 'lvttl'.
    channel_id : int, optional
        The index of the channel to configure, from 0 through 7.

    Returns
    -------
    pulse_params : dict
        The updated pulse parameters dictionary.

    Notes
    -----
    - The function modifies the selected channel's 'output_level' key.

    """
    check_types(
        str,
        level=level
    )

    check_types(
        int,
        channel_id=channel_id
    )

    channels = list(pulse_params)

    if not 0 <= channel_id < len(channels):
        msg = f'Invalid pulse channel selected. Got {channel_id}'
        raise ValueError(msg)

    channel = channels[channel_id]

    if level.lower() not in VALID_OUTPUT_LEVELS:
        msg = f'Invalid pulse output level. Got {level}'
        raise ValueError(msg)

    pulse_params[channel]['output_level'] = level

    return pulse_params