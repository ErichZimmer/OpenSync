__all__ = [
    'get_channel_ids',
    'get_max_pulse',
    'get_used_channels'
]


def get_channel_ids(pulse_params: dict) -> list[str]:
    """Retrieve the list of channel IDs from the pulse parameters.

    This function extracts and returns the channel IDs from the provided
    pulse parameters dictionary.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    keys : list[str]
        A list of channel IDs (keys) from the pulse parameters dictionary.
    """
    return list(pulse_params.keys())[4:]


def get_max_pulse(pulse_params: dict) -> int:
    """Get the maximum pulse value from the pulse parameters.

    This function iterates through the channels in the pulse parameters
    and returns the maximum pulse value found across all channels that
    have more than one pulse entry.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    max_pulse : int
        The maximum pulse value found across all channels. Returns 0 if
        no pulses are found.
    """
    channels = get_channel_ids(pulse_params)
    max_pulse = 0
    
    for channel in channels:
        pulse = pulse_params[channel]['data']
        pulse_len = len(pulse)

        if pulse_len > 1:
            max_val = max(pulse_params[channel]['data'])

            if max_val > max_pulse:
                max_pulse = max_val

    return max_pulse


def get_used_channels(pulse_params: dict) -> int:
    """Count the number of channels that have pulse data.

    This function iterates through the channels in the pulse parameters
    and counts how many channels contain more than one pulse entry.

    Parameters
    ----------
    pulse_params : dict
        A dictionary containing pulse parameters from `get_pulse_params`.

    Returns
    -------
    used_channels : int
        The number of channels that have pulse data entries.
    """
    channels = get_channel_ids(pulse_params)

    used_channels = 0
    
    for channel in channels:
        pulse = pulse_params[channel]['data']
        pulse_len = len(pulse)

        if pulse_len > 1:
            used_channels += 1

    return used_channels