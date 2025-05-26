from copy import deepcopy
from numpy import inf as INF
from typing import Tuple

from ._utils import get_channel_ids
from ..communication import CLOCK_CYCLE


# Simple pulse sequences should not be larger than 256 instructions pairs.
MAX_PULSE_SEQUENCE = 16
EXT_TRIGGER = 0


__all__ = [
    'convert_pulse_params'
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


def _convert_pulse_params(pulse_params: dict) -> Tuple[list[int], list[int]]:  
    output_state_mask = '0000000000000000'
    eps = 0.00001
    
    channels = get_channel_ids(pulse_params)

    # Create a nested list so we can pop output delays as needed
    detected_pulses = []
    for channel in channels:
        pulse = pulse_params[channel]['data']
        if len(pulse) > 1:
            detected_pulses.append(deepcopy(pulse))

    # Copy for coefficients in piece-wise function
    coeffs = deepcopy(detected_pulses)

    output_state = []
    output_delay = []

    min_pulse_len = _get_min_pulse(detected_pulses)
    max_pulse_len = _get_max_pulse(detected_pulses)
    
    if min_pulse_len > eps:
        output_state.append(0)
#        output_delay.append(None) # We will add the final delay later on

    current_delay = 0.0
    total_delay = 0.0
    
    reps = 0
    while reps < MAX_PULSE_SEQUENCE:
        min_pulse_len = _get_min_pulse(detected_pulses)
        current_delay = abs(total_delay - min_pulse_len)
        total_delay += current_delay

        if total_delay > max_pulse_len:
            break
        
        state = list(output_state_mask)
        for channel in range(len(detected_pulses)):
            state[channel] = str(_eval_piecewise_function(
                coeffs[channel], 
                total_delay
            ))

            # Remove lowest delay to progress pointer
            if len(detected_pulses[channel]) > 0:
                if detected_pulses[channel][0] == total_delay:
                    detected_pulses[channel].pop(0)
                    
        output_state.append(int(''.join(state), base=2)) # binary -> decimal
        output_delay.append(current_delay)
            
        reps += 1

    # Remove first delay if min delay is zero
    if min_pulse_len == INF:
        output_delay.pop(0)
        
    # Add null value to signal we still need the final delay
    output_delay.append(None)

    return output_state, output_delay


def _micros_to_nano(microsecond: float) -> int:
    return int(microsecond * 1000)


def _nanos_to_cycle(nanosecond: int) -> int:
    return nanosecond // CLOCK_CYCLE


def _output_delay_cycles(output_delays: list[float]) -> list[int]:
    output_delay_nanos = [_micros_to_nano(delay) for delay in output_delays]
    output_delay_cycels = [_nanos_to_cycle(delay) for delay in output_delay_nanos]

    return output_delay_cycels
    

def _reps_khz_to_delay_micros(
    reps_khz: float,
    output_delay_microseconds: list[float]
) -> float:
    # Just in case, but it should never raise unless something bad happened
    if None not in output_delay_microseconds:
        raise ValueError(
            'Pulse sequence already has repetition delay'
        )
        
    total_delay_microseconds = sum(output_delay_microseconds[:-1])
    delay_reps_microsecond = (1 / reps_khz) * 1000

    return delay_reps_microsecond - total_delay_microseconds

    
def convert_pulse_params(pulse_params: dict) -> Tuple[list[int], list[int]]:
    """Convert pulse parameters into output states and delays.

    This function takes a dictionary of pulse parameters and converts them
    into two lists: one representing the output states for each pulse and
    another representing the corresponding delays in cycles. The conversion
    includes handling external trigger settings and calculating delays based
    on the sequence repetition rate.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    output_state : list[int]
        A list of integers representing the output states for each pulse.
    output_delay : list[int]
        A list of integers representing the corresponding delays in cycles.
    """
    output_state, output_delay_microseconds = _convert_pulse_params(pulse_params)

    output_delay_reps_micros = _reps_khz_to_delay_micros(
        pulse_params['reps_khz'],
        output_delay_microseconds
    )

    output_delay_microseconds[-1] = output_delay_reps_micros

    output_delay_cycles = _output_delay_cycles(output_delay_microseconds)
    
    if pulse_params['ext_trigger'] == 'enabled':
        # Trigger flag needs to be the first instruction
        output_state.insert(0, EXT_TRIGGER)
        output_delay_cycles.insert(0, EXT_TRIGGER)
        
        # Trigger delay needs to be the second instruction
        output_state.insert(1, EXT_TRIGGER)
        output_delay_cycles.insert(1, pulse_params['ext_trigger_delay'])

    return output_state, output_delay_cycles