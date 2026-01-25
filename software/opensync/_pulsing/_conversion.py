from copy import deepcopy
from numpy import inf as INF
from typing import Tuple

from ._utils import _get_channel_ids
from .._error_handles import DeviceBufferSizeError

import warnings


# Simple pulse sequences should not be larger than 10 instructions pairs.
MAX_PULSE_INSTRUCTION_PAIRS = 15 # 15 * 2 = 30 total instructions supported + 2 term. flags
NULL_PULSE_DELAY = 0
DEFAULT_OUTPUT_STATE = 0


__all__ = [
    '_convert_clock_inst',
    '_convert_pulse_inst'
]


def _get_min_pulse(pulse_data) -> float:
    min_pulse = INF
    
    for pulse in pulse_data:
        pulse_len = len(pulse)

        if pulse_len > 0:
            min_val = min(pulse)

            if min_val < min_pulse:
                min_pulse = min_val

    return min_pulse


def _get_max_pulse(pulse_data) -> float:
    max_pulse = 0
    
    for pulse in pulse_data:
        pulse_len = len(pulse)

        if pulse_len > 0:
            max_val = max(pulse)

            if max_val > max_pulse:
                max_pulse = max_val

    return max_pulse


def _eval_piecewise_function(pulse_data: list, x: float) -> bool:
    for pulse in range(0, len(pulse_data), 2):
        pulse_start, pulse_end = pulse_data[pulse], pulse_data[pulse + 1]

        condition = pulse_end > x >= pulse_start

        if condition:
            return 1
    return 0


def _convert_pulse_params(pulse_params: dict) -> Tuple[list[int], list[float]]:  
    OUTPUT_STATE_MASK = '00000000'
    eps = 0.00001
    
    channels = _get_channel_ids(pulse_params)

    # Create a nested list so we can pop output delays as needed
    detected_pulses = []
    for channel in channels:
        pulse = pulse_params[channel]['data']
        if len(pulse) > 1:
            detected_pulses.append(deepcopy(pulse))

    # Copy for coefficients in piece-wise function
    coeffs = deepcopy(detected_pulses)

    output_states = []
    output_delays = []

    min_pulse_len = _get_min_pulse(detected_pulses)
    max_pulse_len = _get_max_pulse(detected_pulses)

    current_delay = 0.0
    total_delay = 0.0
    
    reps = 0
    while reps < MAX_PULSE_INSTRUCTION_PAIRS:
        min_pulse_len = _get_min_pulse(detected_pulses)
        current_delay = abs(total_delay - min_pulse_len)
        total_delay += current_delay

        if total_delay > max_pulse_len:
            break
        
        state = list(OUTPUT_STATE_MASK)
        for channel in range(len(detected_pulses)):
            state[channel] = str(_eval_piecewise_function(
                coeffs[channel], 
                total_delay
            ))

            # Remove lowest delay to progress pointer
            if len(detected_pulses[channel]) > 0:
                if detected_pulses[channel][0] == total_delay:
                    detected_pulses[channel].pop(0)
                    
        output_states.append(int(''.join(state), base=2)) # binary -> decimal
        output_delays.append(current_delay)
            
        reps += 1

    # Raise error about resource management
    if reps > MAX_PULSE_INSTRUCTION_PAIRS:
        msg = 'Device resources has been exausted. The device only supports ' +\
             f'{MAX_PULSE_INSTRUCTION_PAIRS} instructions pairs'
        
        raise DeviceBufferSizeError(msg)

    return output_states, output_delays


def _conversion_delay_bugfix(
    output_delays: list[int]
):
    # If the first delay is zero, replace it (byproduct of the conversion)
    if output_delays[0] == 0:
        output_delays = output_delays[1:]
        output_delays.append(NULL_PULSE_DELAY)

    return output_delays


def _convert_pulse_inst(pulse_params: dict) -> Tuple[list[int], list[float]]:
    """Convert pulse parameters into output states and delays.

    This function takes a dictionary of pulse parameters and converts them
    into two lists: one representing the output states for each pulse and
    another representing the corresponding delays. The actual conversion to
    clock cycles and output instructions occur on device.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    output_states : list[int]
        A list of integers representing the output states for each pulse.
    output_delays : list[float]
        A list of floats representing the corresponding delays.
    """
    output_states, output_delays = _convert_pulse_params(pulse_params)
    
    output_delays = _conversion_delay_bugfix(output_delays)

    return output_states, output_delays


def _convert_clock_inst(clock_params: dict) -> Tuple[list[int], list[int]]:
    """Convert clock parameters into reps and delays.

    This function takes a dictionary of clock parameters and converts them
    into two lists: one representing the repitions for each pulse and
    another representing the corresponding delays in cycles. The conversion
    includes handling external trigger settings and calculating delays based
    on the sequence repetition rate.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    reps : list[int]
        A list of integers representing the repititions for each pulse.
    freqs : list[float]
        A list of frequencies representing the corresponding delays.
    """
    # NOTE: only single instructions are used for now
    reps = clock_params['reps_iter']
    freqs = clock_params['reps_freq']

    return reps, freqs