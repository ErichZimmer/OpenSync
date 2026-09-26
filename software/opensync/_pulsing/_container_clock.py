from .._input_checker import check_types


VALID_TRIGGER_MODES = [
    'disabled',
    'triggered'
]
VALID_TRIGGER_EDGES = [
    'rising',
    'falling'
]
VALID_GATE_MODES = [
    'disabled',
    'output',
    'pulse',
    'channel'
]
VALID_GATE_LEVELS = [
    'high',
    'low'
]

COUNTERS_MAX = 1000000000
BCOUNTER_MAX = 30000000
DIVIDER_MAX = 65500
PERIOD_MIN = 320e-9
PERIOD_MAX = 16


__all__ = [
    'get_clock_params',
    'config_clock_state',
    'config_clock_divider',
    'config_clock_period',
    'config_clock_bcounter',
    'config_clock_pcounter',
    'config_clock_ocounter',
    'config_clock_trigger_mode',
    'config_clock_trigger_edge',
    'config_clock_gate_mode',
    'config_clock_gate_level'
]


def get_clock_params() -> dict:
    """Retrieve the default clock parameters for the pulse generator.

    This function initializes and returns a dictionary containing the
    default clock parameters for the T0 clock sequencer.

    Returns
    -------
    clock_params : dict
        A dictionary containing the default clock parameters. The structure
        of the dictionary includes:
        - 'state' : bool
            Whether the clock sequencer is enabled (default is False).
        - 'divider' : int
            The clock divider (default is 1).
        - 'period' : float
            The clock period in seconds (default is 0.0).
        - 'bcounter' : int
            The number of acquisitions. A value of 0 selects infinite
            operation (default is 0).
        - 'pcounter' : int
            The number of ON periods per acquisition (default is 1).
        - 'ocounter' : int
            The number of OFF periods per acquisition (default is 0).
        - 'trigger_mode' : str
            The external trigger mode (default is 'disabled').
        - 'trigger_edge' : str
            The external trigger edge (default is 'rising').
        - 'gate_mode' : str
            The global gate mode (default is 'disabled').
        - 'gate_level' : str
            The active gate input level (default is 'low').

    Notes
    -----
    - Defaults follow the SCPI reset configuration.
    - Set a valid period before starting the clock sequencer.

    """
    clock_params = {
        'state': False,
        'divider': 1,
        'period': 0.0,
        'bcounter': 0,
        'pcounter': 1,
        'ocounter': 0,
        'trigger_mode': 'disabled',
        'trigger_edge': 'rising',
        'gate_mode': 'disabled',
        'gate_level': 'low'
    }

    return clock_params


def config_clock_state(
    clock_params: dict,
    state: bool
) -> dict:
    """Configure the clock sequencer state.

    This function updates the clock parameters to enable or disable the
    clock sequencer.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    state : bool
        True enables the clock sequencer. False disables it.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'state' key in the clock_params dictionary
      to reflect the desired clock sequencer state.

    """
    check_types(
        bool,
        state=state
    )

    clock_params['state'] = state

    return clock_params


def config_clock_divider(
    clock_params: dict,
    divider: int = 1
) -> dict:
    """Configure the clock divider.

    This function updates the clock parameters by modifying the clock
    divider. The divider scales the clock resolution and the minimum and
    maximum clock period.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    divider : int
        The desired clock divider, from 1 through 65500.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'divider' key in the clock_params
      dictionary to reflect the desired clock divider.

    """
    check_types(
        int,
        divider=divider
    )

    if divider < 1 or divider > DIVIDER_MAX:
        msg = f'Invalid clock divider. Got {divider}'
        raise ValueError(msg)

    clock_params['divider'] = divider

    return clock_params


def config_clock_period(
    clock_params: dict,
    period: float,
    units: str = 's'
) -> dict:
    """Configure the clock period.

    This function converts the supplied period to seconds and updates the
    clock parameters.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    period : float
        The desired clock period in the selected units.
    units : str
        The period units: 'ns', 'us', 'ms', or 's'.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'period' key in the clock_params dictionary
      to store the clock period in seconds.
    - The allowed period is from 320 nanoseconds multiplied by the clock
      divider through 10 seconds multiplied by the clock divider.
    - Configure the divider before configuring the period.

    """
    check_types(
        (float, int),
        period=period
    )
    check_types(
        str,
        units=units
    )

    unit_divisors = {
        'ns': 1e9,
        'us': 1e6,
        'ms': 1e3,
        's': 1.0
    }

    if units.lower() not in unit_divisors:
        msg = f'Invalid clock period units. Got {units}'
        raise ValueError(msg)

    period = period / unit_divisors[units.lower()]

    if not (
        period >= PERIOD_MIN * clock_params['divider'] and
        period <= PERIOD_MAX * clock_params['divider']
    ):
        msg = f'Invalid clock period in seconds. Got {period}'
        raise ValueError(msg)

    clock_params['period'] = period

    return clock_params


def config_clock_bcounter(
    clock_params: dict,
    count: int
) -> dict:
    """Configure the clock acquisition counter.

    This function updates the clock parameters to set the number of
    acquisitions. Each acquisition contains the configured P and O counts.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    count : int
        The desired number of acquisitions, from 0 through 30000000.
        A value of 0 selects infinite operation.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'bcounter' key in the clock_params
      dictionary to store the number of acquisitions.

    """
    check_types(
        int,
        count=count
    )

    if count < 0 or count > BCOUNTER_MAX:
        msg = f'Invalid clock B counter. Got {count}'
        raise ValueError(msg)

    clock_params['bcounter'] = count

    return clock_params


def config_clock_pcounter(
    clock_params: dict,
    count: int
) -> dict:
    """Configure the clock ON duty cycle counter.

    This function updates the clock parameters to set the number of ON
    periods in each acquisition.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    count : int
        The desired number of ON periods, from 0 through 1000000000.
        Use a count of at least 1 for T0 as recommended by the SCPI
        documentation.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'pcounter' key in the clock_params
      dictionary to store the number of ON periods.

    """
    check_types(
        int,
        count=count
    )

    if count < 0 or count > COUNTERS_MAX:
        msg = f'Invalid clock P counter. Got {count}'
        raise ValueError(msg)

    clock_params['pcounter'] = count

    return clock_params


def config_clock_ocounter(
    clock_params: dict,
    count: int
) -> dict:
    """Configure the clock OFF duty cycle counter.

    This function updates the clock parameters to set the number of clock
    periods skipped after the configured ON periods.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    count : int
        The desired number of OFF periods, from 0 through 1000000000.
        A value of 0 means no clock periods are skipped.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'ocounter' key in the clock_params
      dictionary to store the number of OFF periods.

    """
    check_types(
        int,
        count=count
    )

    if count < 0 or count > COUNTERS_MAX:
        msg = f'Invalid clock O counter. Got {count}'
        raise ValueError(msg)

    clock_params['ocounter'] = count

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

        'disabled'
            External triggering is disabled.

        'triggered'
            The selected external trigger edge starts one acquisition
            containing the configured P and O counts.

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
    be used when external triggering is enabled.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    edge : str
        The selected trigger edge. The following are accepted values:

        'rising'
            The trigger system uses the rising edge.

        'falling'
            The trigger system uses the falling edge.

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


def config_clock_gate_mode(
    clock_params: dict,
    gate_mode: str
) -> dict:
    """Configure the global gate mode.

    This function updates the clock parameters to select how the external
    gate affects program execution.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    gate_mode : str
        A string that describes the gate mode. The following are accepted
        values:

        'disabled'
            Global gating is disabled.

        'output'
            An asserted gate suppresses T0 output events while the P/O
            counters continue.

        'pulse'
            An asserted gate prevents a new acquisition from starting.
            An acquisition already started continues.

        'channel'
            Gating is controlled by each output channel's gate settings.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'gate_mode' key in the clock_params
      dictionary to reflect the desired gate mode.

    """
    check_types(
        str,
        gate_mode=gate_mode
    )

    if gate_mode.lower() not in VALID_GATE_MODES:
        msg = f'Invalid gate mode. Got {gate_mode}'
        raise ValueError(msg)

    clock_params['gate_mode'] = gate_mode

    return clock_params


def config_clock_gate_level(
    clock_params: dict,
    level: str
) -> dict:
    """Configure the active gate input level.

    This function updates the clock parameters to select which input level
    asserts the global gate.

    Parameters
    ----------
    clock_params : dict
        A dictionary containing clock parameters from `get_clock_params`.
    level : str
        The selected gate level. The following are accepted values:

        'high'
            The gate is asserted when the input is high.

        'low'
            The gate is asserted when the input is low.

    Returns
    -------
    clock_params : dict
        The updated clock parameters dictionary.

    Notes
    -----
    - The function modifies the 'gate_level' key in the clock_params
      dictionary to reflect the desired gate level configuration.

    """
    check_types(
        str,
        level=level
    )

    if level.lower() not in VALID_GATE_LEVELS:
        msg = f'Invalid gate level. Got {level}'
        raise ValueError(msg)

    clock_params['gate_level'] = level

    return clock_params