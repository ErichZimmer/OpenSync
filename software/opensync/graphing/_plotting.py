from matplotlib import pyplot as plt
import numpy as np

from ..pulsing import _get_channel_ids, _get_max_pulse, _get_used_channels


__all__ = [
    'get_pulse_plot'
]


def _plot_pulses(
    pulse_params: dict,
    axis: 'matplotlib.axis',
    skip_empty: bool=True
) -> None:
    offset = 0.1
    
    channels = _get_channel_ids(pulse_params)
    max_pulse = _get_max_pulse(pulse_params)

    for i, channel in enumerate(channels):
        pulse = pulse_params[channel]['data']
        pulse_len = len(pulse)

        # If there is not pulse data, skip that channel
        if pulse_len < 1 and skip_empty == True:
            continue
        
        states = np.zeros(pulse_len * 2 + 2, dtype=float)
        
        # Iterate through states to get rise and fall edges
        j = 1
        for _ in range(pulse_len // 2):
            states[j + 0 : j + 2] = (np.arange(2) % 2)
            states[j + 2 : j + 4] = ((np.arange(2) + 1) % 2)

            j += 4

        states[states < 0.5] = 0
        states[states > 0.5] = 1 - offset
        
        channel_state = states + i

        channel_signal = np.zeros(pulse_len * 2 + 2)
        channel_signal[1:-1] = np.repeat(pulse, 2)
        channel_signal[-1] = max_pulse

        axis.plot(
            channel_signal, 
            channel_state,
            label=pulse_params[channel]['name']
        )


def get_pulse_plot(
    pulse_params: dict, 
    axis: 'matplotlib.axis',
    skip_empty: bool=False
) -> None:
    """Generate a plot of pulse timing for each channel.

    This function takes pulse timing data for multiple channels and generates
    a plot that visually represents the timing of pulses for each channel
    on the provided Matplotlib axis. Each channel's pulse timing is displayed
    on the y-axis, while the x-axis represents time.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.
    axis : 'matplotlib.axis'
        A Matplotlib axis object on which the pulse timing plot will be drawn.
    skip_empty : bool
        Skip plotting empty channels.

    Returns
    -------
    None
        This function does not return a value. It modifies the provided axis
        to display the pulse timing plot.

    Notes
    -----
    When skipping unused channels, the unused channels must start at channel 0
    and increase from there. Not starting from channel 0 causes the plots to
    appear funky.
    
    """
    axis.set_title('Pulse Timing')
    axis.set_xlabel('Time (microseconds)')
    
    axis.tick_params(
        which='minor',
        bottom=True,
        top=False,
        left=False,
        right=False
    )

    # Set y label tick markers to channel names
    if skip_empty:
        y_labels = [pulse_params[channel]['name'] for channel in _get_used_channels(pulse_params)]
    else:
        y_labels = [pulse_params[channel]['name'] for channel in _get_channel_ids(pulse_params)]
    
    axis.set_yticks(range(len(y_labels)))
    axis.set_yticklabels(y_labels)
    
    _plot_pulses(pulse_params, axis, skip_empty)