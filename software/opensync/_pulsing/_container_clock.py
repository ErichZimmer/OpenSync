from .._input_checker import check_types


VALID_CLOCK_IDS = [0, 1, 2]
VALID_TRIGGER_IDS = [0]
VALID_CLOCK_DIVIDERS = [
    'high',          'high_res',
    'med',           'med_res',
    'low',           'low_res',
    'very_low',      'very_low_res',
    'very_very_low', 'very_very_low_res'
]
VALID_CLOCK_UNITS = [
    'hz',
    'khz',
    'mhz'
]
VALID_CLOCK_MODES = [
    'int', 'internal',
    'ext', 'external'
]
VALID_TRIGGER_MODES = [
    'imm', 'immediate',
    'edge',
    'gate'
]
VALID_TRIGGER_EDGES = [
    'pos', 'positive',
    'neg', 'negative'
]
VALID_TRIGGER_LEVELS = [
    'high',
    'low'
]
VALID_PULSE_UNITS = [
    'ns',
    'us',
    'ms',
    's',
    'm',
    'h'
]
MAX_CLOCK_INST = 16
MAX_ITERATIONS = 500000
MIN_ITERATIONS = 1
MIN_FREQUENCY = 0
MIN_DELAY = 0
MAX_SKIPS = 100
MIN_SKIPS = 0


__all__ = [
    'get_clock_params',
    'config_clock_id',
    'config_clock_res',
    'config_clock_mode',
    'config_clock_units',
    'config_clock_freq',
    'config_clock_iter',
    'config_clock_trigger_id',
    'config_clock_trigger_mode',
    'config_clock_trigger_edge',
    'config_clock_trigger_level',
    'config_clock_trigger_units',
    'config_clock_trigger_skips',
    'config_clock_trigger_delay',
    'config_clock_trigger_count'
]


def get_clock_params() -> dict:
    """Retrieve the default clock parameters for the pulse generator.

    This function initializes and returns a dictionary containing the
    default clock parameters for a pulse generator. The dictionary includes
    settings such as the clock divider, clock mode, external trigger
    configuration, repetition rate, and trigger count.

    Returns
    -------
    clock_params : dict
        A dictionary containing the default clock parameters. The structure
        of the dictionary includes:
        - 'clock_id' : int
            An integer between 0 and 2 representing the clock channel to
            use (default is 0).
        - 'clock_res' : str
            A string representing the clock divider (default is high_res).
        - 'clock_mode' : str
            A string indicating whether the clock channel will run from the
            internal clock sequence or external sequencer/trigger system
            (default is 'internal').
        - 'clock_units' : str
            A string representing the repetition rate units.
        - 'clock_freq' : list[float]
            A list containing the repetition rates in the selected units.
        - 'clock_iter' : list[int]
            A list containing the number of repetitions for each frequency.
        - 'trigger_id' : int
            An integer representing the external trigger input channel to use
            (default is 0).
        - 'trigger_mode' : str
            A string indicating how the external trigger system is used
            (default is 'immediate').
        - 'trigger_edge' : str
            A string indicating which external trigger edge is used when
            trigger mode is edge based (default is 'positive').
        - 'trigger_level' : str
            A string indicating which external trigger gate level is active
            when trigger mode is gate based (default is 'high').
        - 'trigger_units' : str
            A string representing the trigger delay units.
        - 'trigger_skips' : int
            An integer respresenting the number of external trigger signals
            to skip (default is 0).
        - 'trigger_delay' : float
            A float respresenting the delay between the external trigger and
            the start of the pulse sequence (default is 0.0).
        - 'trigger_count' : int
            An integer representing the number of trigger events to accept
            (default is 10).

    """
    clock_params = {
        'clock_id': 0,
        'clock_res': 'high_res',
        'clock_mode': 'internal',
        'clock_units': 'hz',
        'clock_freq': [1.0],
        'clock_iter': [10],
        'trigger_id': 0,
        'trigger_mode': 'immediate',
        'trigger_edge': 'positive',
        'trigger_level': 'high',
        'trigger_units': 'us',
        'trigger_skips': 0,
        'trigger_delay': 0.0,
        'trigger_count': 10
    }

    return clock_params


def config_clock_id(
    clock_params: dict,
    channel_id: int = 0
) -> dict:
    """Configure which clock channel to use.

    This function updates the clock pararameters by selecting which clock
    channel to use for the internal timing and execution of the pulse
    parameters.

    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    channel_id : int
        The index of the clock channel to which the pulse parameters will be
        controlled. Valid clock ids are 0, 1, and 2.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'clock_id' key in the clock_params
      dictionary to reflect the desired clock channel to configure.

    """
    check_types(
        int,
        channel_id=channel_id
    )

    if channel_id not in VALID_CLOCK_IDS:
        msg = f'Invalid clock channel selected. Got {channel_id}'
        raise ValueError(msg)

    clock_params['clock_id'] = channel_id

    return clock_params


def config_clock_res(
    clock_params: dict,
    clock_res: str = 'high_res'
) -> dict:
    """Configure the clock resolution for a clock channel.

    This function updates the clock pararameters by modfying the clock
    divider of that particular channel. This effectively slows the clock down
    by dividing the main clock by the clock divider.

    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    clock_res : str
        The clock divider resolution. The following are accepted values:

        'high', 'high_res'
            The clock divider is set to 1 effectively allowing for a
            clock cycle resolution of 4 nanoseconds.

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
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'clock_res' key in the clock_params
      dictionary to reflect the desired resolution of the clock channel.

    """
    check_types(
        str,
        clock_res=clock_res
    )

    if clock_res.lower() not in VALID_CLOCK_DIVIDERS:
        msg = f'Invalid clock divider resolution. Got {clock_res}'
        raise ValueError(msg)

    clock_params['clock_res'] = clock_res

    return clock_params


def config_clock_mode(
    clock_params: dict,
    clock_mode: str
) -> dict:
    """Configure the clock operation mode.

    This function updates the clock parameters to select whether the clock
    sequencer uses the internal clock sequence or the external sequencer /
    trigger configuration.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    clock_mode : str
        A string that describes the clock sequencer mode. The following are
        accepted values:

        'int', 'internal'
            The clock sequencer runs from the internally configured clock
            sequence.

        'ext', 'external'
            The clock sequencer runs from the external sequencer / trigger
            configuration.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'clock_mode' key in the clock_params
      dictionary to reflect the desired state of the clock channel.

    """
    check_types(
        str,
        clock_mode=clock_mode
    )

    if clock_mode.lower() in VALID_CLOCK_MODES:
        clock_params['clock_mode'] = clock_mode
    else:
        msg = f'Invalid clock mode. Got {clock_mode}'
        raise ValueError(msg)

    return clock_params


def config_clock_units(
    clock_params: dict,
    units: str
) -> dict:
    """Configure repetition rate frequency units.

    This function updates the clock parameters to set the repetition rate
    units for the timing system.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    units : str
        The desired repetition rate unit. The following are accpeted values:

        'hz'
            The frequency unit for standard Hertz (Hz).

        'khz'
            The frequency unit for 1,000 Hz.

        'mhz'
            The frequency unit for 1,000,000 Hz.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'clock_units' key in the clock_params
      dictionary to store the repetition rate units.

    """
    check_types(
        str,
        units=units
    )

    if units.lower() not in VALID_CLOCK_UNITS:
        msg = f'Invalid sequence repetition frequency units. Got {units}'
        raise ValueError(msg)

    clock_params['clock_units'] = units

    return clock_params


def config_clock_freq(
    clock_params: dict,
    freq: float
) -> dict:
    """Configure repetition rate frequency.

    This function updates the clock parameters to set the repetition rate
    for the timing system in the chosen units.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    freq : float
        The desired repetition rate in the chosen units.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'clock_freq' key in the clock_params dictionary
      to store the repetition rate in the chosen units.

    """
    check_types(
        (float, int),
        freq=freq
    )

    if freq < MIN_FREQUENCY:
        msg = f'Invalid sequence repetition frequency. Got {freq}'
        raise ValueError(msg)

    # Make it a list since OpenSync devices support multiple frequencies.
    clock_params['clock_freq'] = [freq]

    return clock_params


def config_clock_iter(
    clock_params: dict,
    iterations: int
) -> dict:
    """Configure number of clock iterations.

    This function updates the clock parameters to set the number of clock
    iterations during program execution.

    Parameters
    ----------
     clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    iterations : int
        The desired number of iterations a pulse sequence is executed.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'clock_iter' key in the clock_params dictionary
      to store the number of clock sequence repetitions.

    """
    check_types(
        int,
        iterations=iterations
    )

    if iterations < MIN_ITERATIONS or iterations > MAX_ITERATIONS:
        msg = f'Invalid number of sequence iterations. Got {iterations}'
        raise ValueError(msg)

    # Make it a list since OpenSync devices support multiple frequencies.
    clock_params['clock_iter'] = [iterations]

    return clock_params


def config_clock_trigger_id(
    clock_params: dict,
    channel_id: int = 0
) -> dict:
    """Configure which trigger input channel to use.

    This function updates the clock pararameters by selecting which external
    trigger input channel to use for the internal timing and execution of the
    pulse parameters.

    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    channel_id : int
        The index of the trigger input channel to use. Valid trigger ids are
        currently 0.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_id' key in the clock_params
      dictionary to reflect the desired trigger input channel to configure.

    """
    check_types(
        int,
        channel_id=channel_id
    )

    if channel_id not in VALID_TRIGGER_IDS:
        msg = f'Invalid trigger channel selected. Got {channel_id}'
        raise ValueError(msg)

    clock_params['trigger_id'] = channel_id

    return clock_params


def config_clock_trigger_mode(
    clock_params: dict,
    trigger_mode: str
) -> dict:
    """Configure the external trigger mode.

    This function updates the clock parameters to select how the clock
    sequencer's trigger system is controlled.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    trigger_mode : str
        A string that describes the trigger mode. The following are accepted
        values:

        'imm', 'immediate'
            The trigger system immediately allows the configured clock
            sequence to run.

        'edge'
            The trigger system waits for the selected external trigger edge.

        'gate'
            The trigger system is gated by the selected external trigger
            level.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_mode' key in the clock_params
      dictionary to reflect the desired trigger mode.

    """
    check_types(
        str,
        trigger_mode=trigger_mode
    )

    if trigger_mode.lower() not in VALID_TRIGGER_MODES:
        msg = f'Invalid trigger mode. Got {trigger_mode}'
        raise ValueError(msg)

    clock_params['trigger_mode'] = trigger_mode

    return clock_params


def config_clock_trigger_edge(
    clock_params: dict,
    edge: str
) -> dict:
    """Configure the external trigger edge.

    This function updates the clock parameters to select which edge should
    be used when the trigger mode is edge based.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    edge : str
        The selected trigger edge. The following are accepted values:

        'pos', 'positive'
            The trigger system uses the positive/rising edge.

        'neg', 'negative'
            The trigger system uses the negative/falling edge.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_edge' key in the clock_params
      dictionary to reflect the desired edge configuration.

    """
    check_types(
        str,
        edge=edge
    )

    if edge.lower() not in VALID_TRIGGER_EDGES:
        msg = f'Invalid trigger edge. Got {edge}'
        raise ValueError(msg)

    clock_params['trigger_edge'] = edge

    return clock_params


def config_clock_trigger_level(
    clock_params: dict,
    level: str
) -> dict:
    """Configure the external trigger gate level.

    This function updates the clock parameters to select which gate level
    should be active when the trigger mode is gate based.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    level : str
        The selected trigger gate level. The following are accepted values:

        'high'
            The trigger gate is active when the input is high.

        'low'
            The trigger gate is active when the input is low.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_level' key in the clock_params
      dictionary to reflect the desired gate level configuration.

    """
    check_types(
        str,
        level=level
    )

    if level.lower() not in VALID_TRIGGER_LEVELS:
        msg = f'Invalid trigger gate level. Got {level}'
        raise ValueError(msg)

    clock_params['trigger_level'] = level

    return clock_params


def config_clock_trigger_units(
    clock_params: dict,
    units: str = 'us'
) -> dict:
    """Configure the units for a clock trigger delay.

    This function updates the clock pararameters by modfying the units of
    the trigger delay. This makes it easier to produce either very short or
    long delays by simply changing the data units.

    clock_params : dict
        A dictionary containing pulse parameters from `get_clock_params`.
    units : str
        The trigger data units. The following are accepted values:

        'ns'
            The trigger delay data is in nanoseconds.

        'us'
            The trigger delay data is in microseconds.

        'ms'
            The trigger delay data is in milliseconds.

        's'
            The trigger delay data is in seconds.

        'm'
            The trigger delay data is in minutes.

        'h'
            The trigger delay data is in hours.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_units' key in the clock_params
      dictionary to reflect the desired units of the external trigger delay.

    """
    check_types(
        str,
        units=units
    )

    if units.lower() not in VALID_PULSE_UNITS:
        msg = f'Invalid data units. Got {units}'
        raise ValueError(msg)

    clock_params['trigger_units'] = units

    return clock_params


def config_clock_trigger_skips(
    clock_params: dict,
    skips: int
) -> dict:
    """Change the number external triggers to skip.

    This function updates the clock parameters to configure the amount of
    external trigger events to skip before generating a clock event.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    skips : int
        The desired amount of external triggers to skip before executing
        the pulse parameters.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_skips' key in the clock_params
      dictionary to reflect the desired state of the external trigger.

    """
    check_types(
        int,
        skips=skips
    )

    if skips < MIN_SKIPS or skips > MAX_SKIPS:
        msg = f'Invalid external trigger skips. Got {skips}'
        raise ValueError(msg)

    clock_params['trigger_skips'] = skips

    return clock_params


def config_clock_trigger_delay(
    clock_params: dict,
    delay: float
) -> dict:
    """Change external trigger delay.

    This function updates the clock parameters to add a delay to the
    external trigger of the timing system used in the experiment.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    delay : float
        The desired delay in the currently selected trigger units between
        the external trigger and the start of the pulse sequence.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_delay' key in the clock_params
      dictionary to reflect the desired state of the external trigger.

    """
    check_types(
        (float, int),
        delay=delay
    )

    if delay >= MIN_DELAY:
        clock_params['trigger_delay'] = delay
    else:
        msg = f'Invalid external trigger delay. Got {delay}'
        raise ValueError(msg)

    return clock_params


def config_clock_trigger_count(
    clock_params: dict,
    count: int
) -> dict:
    """Configure number of trigger events.

    This function updates the clock parameters to set the amount of trigger
    events to be accepted during program execution.

    Parameters
    ----------
     clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    count : int
        The desired number of trigger events to accept.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'trigger_iter' key in the clock_params
      dictionary to reflect the desired number of trigger aquisitions.
    - This helper maps to the SCPI ``TRIGger:CLOCk#:COUNt`` command.

    """
    check_types(
        int,
        count=count
    )

    if count < MIN_ITERATIONS or count > MAX_ITERATIONS:
        msg = f'Invalid number of trigger events. Got {count}'
        raise ValueError(msg)

    clock_params['trigger_count'] = count

    return clock_params
