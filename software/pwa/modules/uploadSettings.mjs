const success = '';

async function uploadSystemSettings(device, user) {
    const systemSettings = user.systemSettings;
    let command = '';
    let resp = '';

    command = `pulse0:state ${systemSettings.enable ? 1 : 0}`;
    resp = await device.send(command);

    if (resp) {
        return `System enable failed: got ${resp} from ${command}`;
    }

    command = `pulse0:divider ${systemSettings.divider}`;
    resp = await device.send(command);

    if (resp) {
        return `System clock divider failed: got ${resp} from ${command}`;
    }

    command = `pulse0:period ${systemSettings.period} ${systemSettings.units}`;
    resp = await device.send(command);

    if (resp) {
        return `System period failed: got ${resp} from ${command}`;
    }

    command = `pulse0:bcounter ${systemSettings.bcounter}`;
    resp = await device.send(command);

    if (resp) {
        return `System burst counter failed: got ${resp} from ${command}`;
    }

    command = `pulse0:pcounter ${systemSettings.pcounter}`;
    resp = await device.send(command);

    if (resp) {
        return `System pulse counter failed: got ${resp} from ${command}`;
    }

    command = `pulse0:ocounter ${systemSettings.ocounter}`;
    resp = await device.send(command);

    if (resp) {
        return `System off counter failed: got ${resp} from ${command}`;
    }

    return success;
}

async function uploadTriggerSettings(device, user) {
    const systemSettings = user.systemSettings;
    let command = '';
    let resp = '';

    command = `pulse0:trigger:mode ${systemSettings.trigger_mode}`;
    resp = await device.send(command);

    if (resp) {
        return `System trigger mode failed: got ${resp} from ${command}`;
    }

    command = `pulse0:trigger:edge ${systemSettings.trigger_edge}`;
    resp = await device.send(command);

    if (resp) {
        return `System trigger edge failed: ${divider}`;
    }

    command = `pulse0:gate:mode ${systemSettings.gate_mode}`;
    resp = await device.send(command);

    if (resp) {
        return `System gate mode failed: got ${resp} from ${command}`;
    }

    command = `pulse0:gate:logic ${systemSettings.gate_level}`;
    resp = await device.send(command);

    if (resp) {
        return `System gate level failed: got ${resp} from ${command}`;
    }

    return success;
}

async function uploadChannelSettings(device, user, channel_id = 0) {
    const channelSettings = user.channelSettings[channel_id];
    const channelNumber = channel_id + 1;
    const pulse = `pulse${channelNumber}`;

    const states = channelSettings.states;
    const delays = channelSettings.delays;
    const units = channelSettings.units;

    if (
        states.length < 1 ||
        states.length > 6 ||
        states.length !== delays.length ||
        states.length !== units.length
    ) {
        return `Channel ${channelNumber} requires 1–6 matching state, delay, and unit entries.`;
    }

    let command = '';
    let resp = '';

    command = `${pulse}:state ${channelSettings.enable ? 1 : 0}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} enable failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:output:level ${channelSettings.mode}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} output mode failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:divider ${channelSettings.divider}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} clock divider failed: got ${resp} from ${command}`;
    }

    // Stored sync values are T0 or A–H.
    // The firmware expects T0 or CHA–CHH.
    let sync = channelSettings.sync;

    if (sync !== 'T0') {
        sync = `CH${sync}`;
    }

    command = `${pulse}:sync ${sync}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} sync failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:wcounter ${channelSettings.wcounter}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} wait counter failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:bcounter ${channelSettings.bcounter}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} burst counter failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:pcounter ${channelSettings.pcounter}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} pulse counter failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:ocounter ${channelSettings.ocounter}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} off counter failed: got ${resp} from ${command}`;
    }

    // Send all state/delay pairs in one command:
    // pulse1:buffer on,15us,off,1ms,...
    const buffer = [];

    for (let index = 0; index < states.length; index++) {
        buffer.push(states[index]);
        buffer.push(`${delays[index]}${units[index]}`);
    }

    command = `${pulse}:buffer ${buffer.join(',')}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} state/delay buffer failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:cgate:mode ${channelSettings.gate_mode}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} gate mode failed: got ${resp} from ${command}`;
    }

    command = `${pulse}:cgate:logic ${channelSettings.gate_level}`;
    resp = await device.send(command);

    if (resp) {
        return `Channel ${channelNumber} gate logic failed: got ${resp} from ${command}`;
    }

    return success;
}

async function uploadSettings(device, user) {
    const numChannels = user.channelSettings.length;

    let resp = '';

    resp = await uploadSystemSettings(device, user);

    if (resp) {
        return resp;
    }

    resp = await uploadTriggerSettings(device, user);

    if (resp) {
        return resp;
    }

    for (let i = 0; i<numChannels; i++) {
        const channelSettings = user.channelSettings[i];

        // If a channel is not enabled, don't bother uploading the settings
        if (!channelSettings.enable) {
            continue;
        }

        resp = await uploadChannelSettings(device, user, i);

        if (resp) {
            return resp;
        }
    }

    return success;
}

export default uploadSettings;