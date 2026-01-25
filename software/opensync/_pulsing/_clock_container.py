VALID_CLOCK_IDS = [0, 1, 2]
VALID_CLOCK_DIVIDERS = [
    'high', 'high_res',
    'med', 'med_res',
    'low', 'low_res',
    'very_low', 'very_low_res',
    'very_very_low', 'very_very_low_res'
]
VALID_FREQ_UNITS = [
    'micro', 'microhertz',
    'milli', 'millihertz',
    'hertz',
    'kilo', 'kilohertz',
    'mega', 'megahertz'
]
MAX_CLOCK_INST = 64
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
    'config_trigger',
    'config_trigger_id',
    'config_trigger_skips',
    'config_trigger_delay',
    'config_reps_freq_units',
    'config_reps_freq',
    'config_reps_iter'
]


def get_clock_params() -> dict:
    """Retrieve the default clock parameters for the pulse generator.

    This function initializes and returns a dictionary containing the
    default clock parameters for a pulse generator. The dictionary includes
    settings such as the clock divider, external trigger status, repetition
    rate in kilohertz.

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
        - 'ext_trigger' : str
            A string indicating the external trigger status (default is
            'disabled').
        - 'ext_trigger_id' : int
            An integer between 0 and 2 representing the external trigger
            channel to use (default is 0).
        - 'ext_trigger_skip' : int
            An integer respresenting the number of external trigger signals
            to skip (default is 0).
        - 'ext_trigger_delay' : float
            A float respresenting the delay between the external trigger and
            the start of the pulse sequence (default is 0.0).
        - 'reps_freq_units' : str
            A string representing the repetition rate units.
        - 'reps_freq' : float
            A float representing the repetition rate in the selected units.
        - 'reps_iter' : int
            An integer representing the number of iterations to perform
            (default is 10).
    """
    clock_params = {
        'clock_id': 0,
        'clock_res': 'high_res',
        'ext_trigger': 'disabled',
        'ext_trigger_id': 0,
        'ext_trigger_skips': 0,
        'ext_trigger_delay': 0.0,
        'reps_freq_units': 'hertz',
        'reps_freq': [1.0],
        'reps_iter': [10]
    }
        
    return clock_params


def config_clock_id(
    clock_params: dict,
    channel_id: int=0
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
    
    """
    if channel_id not in VALID_CLOCK_IDS:
        msg = f'Invalid clock channel selected. Got {channel_id}'
        raise ValueError(msg)

    clock_params['clock_id'] = channel_id

    return clock_params
    

def config_clock_res(
    clock_params: dict,
    clock_res: str='high_res'
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
            clock cycle resolution of 4 nanosecondsns.

        'med', 'med_res'
            The clock divider is set to 25 effectively allowing for a
            clock cycle resolution of 100 nanoseconds.

        'low', 'low_res'
            The clock divider is set to 250 effectively allowing for a
            clock cycle resolution of 1 mirocseconds.

        'very_low', 'very_low_res'
            The clock divider is set to 2,500 effectively allowing for a
            clock cycle resolution of 10 mirocseconds.

        'very_very_low', 'very_very_low_res'
            The clock divider is set to 25,000 effectively allowing for a
            clock cycle resolution of 100 mirocseconds.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.
    
    """
    if clock_res.lower() not in VALID_CLOCK_DIVIDERS:
        msg = f'Invalid clock divider resolution. Got {clock_res}'
        raise ValueError(msg)

    clock_params['clock_res'] = clock_res

    return clock_params
    

def config_trigger(
    clock_params: dict,
    enable: bool
) -> dict:
    """Change external trigger configuration.

    This function updates the clock parameters to enable or disable
    the external trigger for the timing system used in the experiment.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    enable : bool
        A boolean value indicating whether to enable or disable the
        external trigger. If True, the external trigger is enabled; if
        False, it is disabled.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.
    
    Notes
    -----
    - The function modifies the 'ext_trigger' key in the clock_params
      dictionary to reflect the desired state of the external trigger.
    """
    if enable == True:
        clock_params['ext_trigger'] = 'enabled'
    elif enable == False:
        clock_params['ext_trigger'] = 'disabled'
    else:
        msg = f'Invalid external trigger status. Got {enable}'
        raise ValueError(msg)

    return clock_params


def config_trigger_id(
    clock_params: dict,
    channel_id: int=0
) -> dict:
    """Configure which trigger channel to use.

    This function updates the clock pararameters by selecting which external
    trigger input channel to use for the internal timing and execution of the
    pulse parameters.

    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    channel_id : int
        The index of the clock channel to which the pulse parameters will be
        controlled. Valid clock ids are 0, 1, and 2.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.
    
    """
    if channel_id not in VALID_CLOCK_IDS:
        msg = f'Invalid clock channel selected. Got {channel_id}'
        raise ValueError(msg)
    
    clock_params['ext_trigger_id'] = channel_id

    return clock_params

    
def config_trigger_skips(
    clock_params: dict,
    skips: int
) -> dict:
    """Change the number external triggers to skip.

    This function updates the clock parameters to add a delay to the
    external trigger of the timing system used in the experiment.

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
    - The function modifies the 'ext_trigger_skips' key in the clock_params
      dictionary to reflect the desired state of the external trigger.
    """
    if skips < MIN_SKIPS or skips > MAX_SKIPS:
        msg = f'Invalid external trigger skips. Got {skips}'
        raise ValueError(msg)
        
    clock_params['ext_trigger_skips'] = skips

    return clock_params
    

def config_trigger_delay(
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
        The desired delay in microseconds between the external trigger and
        the start of the pulse sequence.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.
    
    Notes
    -----
    - The function modifies the 'ext_trigger_delay' key in the clock_params
      dictionary to reflect the desired state of the external trigger.
    """
    if delay >= MIN_DELAY:
        clock_params['ext_trigger_delay'] = delay
    else:
        msg = f'Invalid external trigger delay. Got {delay}'
        raise ValueError(msg)

    return clock_params
        

def config_reps_freq_units(
    clock_params: dict,
    units: float
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

        `micro', 'microhertz'
            The frequency unit for 0.00001 Hz.

        'milli', 'millihertz'
            The frequency unit for 0.001 Hz.

        'hertz'
            The frequency unit for standard Hertz (Hz).

        'kilo', 'kilohertz'
            The frequency unit for 1,000 Hz.

        'mega', 'megahertz'
            The frequency unit for 1,000,000 Hz.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.
    
    Notes
    -----
    - The function modifies the 'reps_freq_units' key in the clock_params
      dictionary to store the repetition rate units.
    """
    if units not in VALID_FREQ_UNITS:
        msg = f'Invalid sequence repitition frequency units. Got {units}'
        raise ValueError(msg)
        
    clock_params['reps_freq_units'] = units
    
    return clock_params


def config_reps_freq(
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
    - The function modifies the 'reps_freq' key in the clock_params dictionary
      to store the repetition rate in the chosen units.
      input hertz value by 1000.
    """
    if freq < MIN_FREQUENCY:
        msg = f'Invalid sequence repitition frequency. Got {freq}'
        raise ValueError(msg)
        
    clock_params['reps_freq'] = [freq]
    
    return clock_params


def config_reps_iter(
    clock_params: dict,
    iterations: int
) -> dict:
    """Configure repetition rate.

    This function updates the clock parameters to set the repetition rate
    for the timing system in hertz (kHz).

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
    - The function modifies the 'reps_khz' key in the clock_params dictionary
      to store the repetition rate in kilohertz, calculated by dividing the
      input hertz value by 1000.
    """
    if iterations < MIN_ITERATIONS or iterations > MAX_ITERATIONS:
        msg = f'Invalid number of sequence iterations. Got {iterations}'
        raise ValueError(msg)
        
    clock_params['reps_iter'] = [iterations]
    
    return clock_params