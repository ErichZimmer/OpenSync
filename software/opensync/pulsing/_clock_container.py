VALID_CLOCK_IDS = [0, 1, 2]
MAX_CLOCK_DIVIDER = 2**16 - 1
MAX_ITERATIONS = 500000
MIN_ITERATIONS = 1
MIN_FREQUENCY = 0
MIN_DELAY = 0
MAX_SKIPS = 100
MIN_SKIPS = 0


__all__ = [
    'get_clock_params',
    'config_clock_id',
    'config_clock_divider',
    'config_trigger',
    'config_trigger_id',
    'config_trigger_skips',
    'config_trigger_delay',
    'config_reps_hz',
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
        - 'clock_id': An integer between 0 and 2 representing the clock
           channel to use (default is 0).
        - 'clock_divider': An integer representing the clock divider
           (default is 1).
        - 'ext_trigger': A string indicating the external trigger status
           (default is 'disabled').
        - 'ext_trigger_id': An integer between 0 and 2 representing the
           external trigger channel to use (default is 0).
        - 'ext_trigger_delay': An integer respresenting the number of ext.
           trigger signals to skip (default is 0).
        - 'ext_trigger_delay': A float respresenting the delay between the
           external trigger and the start of the pulse sequence 
           (default is 0.0).
        - 'reps_khz': A float representing the repetition rate in kilohertz
           (default is 0.01 khz).
        - 'reps_iter': An integer representing the number of iterations to
           perform (default is 10).
    """
    clock_params = {
        'clock_id': 0,
        'clock_divider': 1,
        'ext_trigger': 'disabled',
        'ext_trigger_id': 0,
        'ext_trigger_skips': 0,
        'ext_trigger_delay': 0.0,
        'reps_khz': 0.01,
        'reps_iter': 10
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
    

def config_clock_divider(
    clock_params: dict,
    clock_divider: int=1
) -> dict:
    """Configure the clock divider for a clock channel.

    This function updates the clock pararameters by modfying the clock
    divider of that particular channel. This effectively slows the clock down
    by dividing the main clock by the clock divider.

    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    clock_divider : int
        The amount to divide the main clock. Must be no greater than 2^16-1.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.
    
    """
    if clock_divider < 1 or clock_divider > MAX_CLOCK_DIVIDER:
        msg = f'Invalid clock divider. Got {clock_divider}'
        raise ValueError(msg)

    clock_params['clock_divider'] = clock_divider

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
        

def config_reps_hz(
    clock_params: dict,
    hertz: int
) -> dict:
    """Configure repetition rate.

    This function updates the clock parameters to set the repetition rate
    for the timing system in hertz (kHz).

    Parameters
    ----------
     clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    hertz : int
        The desired repetition rate in hertz (Hz). This value will be
        converted to kilohertz (kHz) for storage in the clock parameters.

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
    if hertz < MIN_FREQUENCY:
        msg = f'Invalid sequence repitition frequency. Got {hertz}'
        raise ValueError(msg)
        
    khz = hertz / 1000
    clock_params['reps_khz'] = khz
    
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
        
    clock_params['reps_iter'] = iterations
    
    return clock_params