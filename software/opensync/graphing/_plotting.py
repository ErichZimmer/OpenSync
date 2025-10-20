from matplotlib import pyplot as plt
import numpy as np

from ..pulsing import _get_channel_ids, _get_max_pulse, _get_used_channels


__all__ = [
    'get_pulse_plot'
]


def _plot_pulses(
    pulse_params: dict,
    axis: 'matplotlib.axis'
) -> None:
    offset = 0.1
    
    channels = _get_channel_ids(pulse_params)
    max_pulse = _get_max_pulse(pulse_params)

    for i, channel in enumerate(channels):
        pulse = pulse_params[channel]['data']
        pulse_len = len(pulse)

        if pulse_len > 1:
            state = np.zeros(pulse_len * 2 + 2, dtype=float)
            
            state[1:3] = (np.arange(2) % 2)
            state[3:5] = ((np.arange(2) + 1) % 2)

            if pulse_len > 2:
                state[5:7] = (np.arange(2) % 2)
                state[7:9] = ((np.arange(2) + 1) % 2)

            state[state < 0.5] = 0
            state[state > 0.5] = 1 - offset
            
            channel_state = state + i

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
    axis: 'matplotlib.axis'
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

    Returns
    -------
    None
        This function does not return a value. It modifies the provided axis
        to display the pulse timing plot.
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
    num_channels = _get_used_channels(pulse_params)
    y_labels = [pulse_params[channel]['name'] for channel in _get_channel_ids(pulse_params)]
    y_labels = y_labels[:num_channels]
    
    axis.set_yticks(range(len(y_labels)))
    axis.set_yticklabels(y_labels)
    
    _plot_pulses(pulse_params, axis)