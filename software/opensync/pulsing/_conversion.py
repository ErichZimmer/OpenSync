from copy import deepcopy
from numpy import inf as INF
from typing import Tuple

from ._utils import get_channel_ids
from ..communication import CLOCK_CYCLE
from ..error_handles import DeviceBufferSizeWarning

import warnings


# Simple pulse sequences should not be larger than 30 instructions pairs.
MAX_PULSE_INSTRUCTION_PAIRS = 31 # 62 total instructions supported + 2 term. flags
MIN_PULSE_CYCLE_WIDTH = 5
DEFAULT_OUTPUT_STATE = 0


__all__ = [
    'convert_clock_inst',
    'convert_pulse_inst'
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
    OUTPUT_STATE_MASK = '000000000000'
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
                    
        output_state.append(int(''.join(state), base=2)) # binary -> decimal
        output_delay.append(current_delay)
            
        reps += 1

    # Warn about resource management
    if reps > MAX_PULSE_INSTRUCTION_PAIRS:
        msg = 'Device resources has been exausted. The device only supports ' +\
             f'{MAX_PULSE_INSTRUCTION_PAIRS} instructions pairs. Remaining ' +\
              'instructions have been ignored'
        
        warnings.warn(
            msg,
            DeviceBufferSizeWarning
        )

    return output_state, output_delay


def _micros_to_nano(microsecond: float) -> int:
    return int(microsecond * 1000)


def _nanos_to_cycle(nanosecond: int) -> int:
    return nanosecond // CLOCK_CYCLE


def _output_delay_cycles(output_delays: list[float]) -> list[int]:
    output_delay_nanos = [_micros_to_nano(delay) for delay in output_delays]
    output_delay_cycels = [int(_nanos_to_cycle(delay)) for delay in output_delay_nanos]

    return output_delay_cycels
    

def _reps_khz_to_delay_micros(
    reps_khz: list[float],
) -> float:
    delay_microseconds = [(1 / delay) * 1000 for delay in reps_khz]

    return delay_microseconds


def _conversion_delay_bugfix(
    output_delay: list[int]
):
    # If the first delay is zero, replace it (byproduct of the conversion)
    if output_delay[0] == 0:
        output_delay = output_delay[1:]
        output_delay.append(MIN_PULSE_CYCLE_WIDTH)

    return output_delay


def convert_pulse_inst(pulse_params: dict) -> Tuple[list[int], list[int]]:
    """Convert pulse parameters into output states and delays.

    This function takes a dictionary of pulse parameters and converts them
    into two lists: one representing the output states for each pulse and
    another representing the corresponding delays in cycles. The conversion
    includes calculating delays based on the sequence repetition rate.

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
    output_delay_cycles = _output_delay_cycles(output_delay_microseconds)
    
    output_delay_cycles = _conversion_delay_bugfix(output_delay_cycles)

    return output_state, output_delay_cycles


def convert_clock_inst(clock_params: dict) -> Tuple[list[int], list[int]]:
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
    delay : list[int]
        A list of integers representing the corresponding delays in cycles.
    """
    # NOTE: only single instructions are used for now
    reps = clock_params['reps_iter']
    delays = clock_params['reps_khz']
    delays_trigger = [clock_params['ext_trigger_delay']]

    delay_micros = _reps_khz_to_delay_micros(delays)
    delay_cycles = _output_delay_cycles(delay_micros)
    trigger_delay_cycles = _output_delay_cycles(delays_trigger)

    return reps, delay_cycles, trigger_delay_cycles[0]