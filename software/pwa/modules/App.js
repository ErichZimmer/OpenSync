import arcaneThemeReady from "arcane-os/modules/ThemeBootstrap.js";
import "arcane-os/modules/HTMLImport.js";
import waitForComponent from "arcane-os/modules/WaitForComponent.js";
import UserEntity, {
    createSystemSettings,
    createChannelSettings,
    SYSTEM_PERIOD_MIN
} from "../entities/User.js";
import uploadSettings from "./uploadSettings.mjs";
import DeviceManager from './connectionManager.mjs';


await arcaneThemeReady;

const user = new UserEntity();
await user.load();

const COMMUNICATION_ERROR = 'Device Communication Error';
const PARAMETER_ERROR =  'Device Parameter Error';

const connectButton = document.querySelector('#connect-device');
const runStopButton = document.querySelector('#run-stop');
const sendButton = document.querySelector('#send-settings');

const systemEditor = document.querySelector(".system-layout");

const channelList    = document.querySelector(".channel-list");
const channelEditor  = document.querySelector("#channel-editor");
const channelHeading = document.querySelector("#channel-editor-heading");

const channelInstructionCount = 6;
const startingChannel = 'A'; // Page refresh always starts on channel A

const device = new DeviceManager();

const terminal = document.createElement('html-import');
const terminalRoot = terminal.shadowRoot;
const terminalURL = new URL(
    '../components/terminal-workspace.html',
    import.meta.resolve('arcane-os/modules/WaitForComponent.js')
);

const modalURL = new URL(
    '../components/modal.html',
    import.meta.resolve('arcane-os/modules/WaitForComponent.js')
);

let terminalBusy = false;
terminal.id = 'scpi-terminal';

terminal.setAttribute('href', terminalURL.href);
document.querySelector('#scpi-terminal-container').append(terminal);

await waitForComponent(terminal, {
    event: 'terminal-workspace-ready',
    property: 'ready',
    methods: ['configure', 'addSession', 'append'],
    errorEvent: 'html-import-error'
});

terminalRoot.querySelector('#tabs').style.display = 'none';
terminalRoot.querySelector('.toolbar').style.display = 'none';

terminalRoot.querySelector('.workspace').style.gridTemplateRows =
    'minmax(0, 1fr) auto';

terminal.configure({
    prompt: 'SCPI >'
});

terminal.addSession({
    id: 'opensync',
    title: 'OpenSync SCPI',
    state: 'ready'
});

async function sendTerminalCommand(event) {
    const command = event.detail.line.trim();

    if (!device.isopen()) {
        terminal.append('Connect to OpenSync first.\n', 'error');
        return;
    }

    if (terminalBusy) {
        terminal.append('Wait for the current command to finish.\n', 'error');
        return;
    }

    terminalBusy = true;

    try {
        const response = await device.send(command);

        if (response === '') {
            terminal.append('OK\n');
        } else {
            terminal.append(response + '\n');
        }
    } catch (error) {
        terminal.append(error.message + '\n', 'error');
    } finally {
        terminalBusy = false;
    }
}

async function showMessage(message, title='OpenSync') {
    const modal = document.createElement('html-import');

    modal.setAttribute('href', modalURL.href);
    modal.setAttribute('data-once', '');

    const content = document.createElement('section');
    const heading = document.createElement('h2');
    const text    = document.createElement('p');

    heading.textContent = title;
    text.textContent = message;
    text.style.whiteSpace = 'pre-wrap';

    content.append(heading, text);

    const closed = new Promise(function (resolve) {
        modal.addEventListener('modal-closed', resolve, { once: true });
    });

    document.body.append(modal);

    try {
        await waitForComponent(modal, {
            event: 'modal-ready',
            property: 'ready',
            methods: ['populate', 'open'],
            errorEvent: 'html-import-error'
        });

        await modal.populate(content, false);
        await modal.open();
        await closed;
    } catch (error) {
        modal.remove();
        throw error;
    }
}

populateSystem();
populateChannels(startingChannel);

function updateConnectionButton() {
    if (device.isopen()) {
        connectButton.textContent = 'Disconnect';
    } else {
        connectButton.textContent = 'Connect';
    }
}

async function manageConnection() {
    console.log('Connecting/disconnecting device');
    connectButton.disabled = true;
    runStopButton.disabled = true;

    try {
        if (device.isopen()) {
            await device.close();
        } else {
            await device.open();
        }
    } catch (err) {
        // TODO: Add a modal to show connection issue
        console.error('Connection failed:', err);
    } finally {
        updateConnectionButton();

        connectButton.disabled = false;
        runStopButton.disabled = false;
    }
}

async function manageRunStop() {
    if (!device.isopen()) {
        const msg = 'Device failed to communicate. Make sure that it is connected'
        await showMessage(msg, COMMUNICATION_ERROR)

        console.error('Connect to OpenSync before using Run/Stop.');

        return;
    }

    runStopButton.disabled = true;
    connectButton.disabled = true;

    try {
        const status = (await device.send('device:status?')).trim();
        let command = '';
        let message = '';

        console.log(`Device status: ${status}`);

        if (status === 'RUNNING') {
            console.log('Stopping device');
            command = 'device:stop';
            message = 'Stopping OpenSync device via abort command'
        } else {
            console.log('Starting device')
            command = 'device:start';
            message = 'Starting OpenSync device'
        }

        const response = await device.send(command);

        if (response) {
            throw new Error(response);
        }

        const postResponseStatus = (await device.send('device:status?')).trim();
        
        await showMessage(message + `\nDevice status: ${postResponseStatus}`);
        console.log(`Device status post command: ${postResponseStatus}`);

    } catch (err) {
        console.error('Run/Stop failed:', err);
    } finally {
        runStopButton.disabled = false;
        connectButton.disabled = false;
    }
}

async function sendSettingsToDevice() {
    if (!device.isopen()) {
        const msg = 'Device failed to communicate. Make sure that it is connected';
        await showMessage(msg, COMMUNICATION_ERROR);

        console.error('Connect to OpenSync before using Run/Stop.');

        return;
    }

    const result = await uploadSettings(device, user);

    console.log(`Result of send: ${result}`);

    let msg = '';
    let title = 'OpenSync';

    if (result) {
        msg += result
        title = PARAMETER_ERROR;
    } else {
        msg = 'Settings sucessfully uploaded to device';
    }

    await showMessage(msg, title);
}

function mapChannelToIndex(channel='') {
    if (!channel) {
        const msg = 'Channel name must be specified';
        throw new Error(msg);
    }

    switch (channel) {
        case 'A':
            return 0;
        case 'B':
            return 1;
        case 'C':
            return 2;
        case 'D':
            return 3;
        case 'E':
            return 4;
        case 'F':
            return 5;
        case 'G':
            return 6;
        case 'H':
            return 7;
        default:
            break;
    }

    // If we reach this point, then the wrong string was passed.
    throw new Error('Invalid channel id')
}

function populateChannels(channel='') {
    if (!channel) {
        const msg = 'Channel name must be specified';
        throw new Error(msg);
    }

    const channelIndex = mapChannelToIndex(channel);
    const channelSettings = user.channelSettings[channelIndex];

    channelEditor.querySelector('[name="enable"]').checked     = channelSettings.enable;
    channelEditor.querySelector('[name="mode"]').value         = channelSettings.mode;
    channelEditor.querySelector('[name="divider"]').value      = channelSettings.divider;
    channelEditor.querySelector('[name="waitCount"]').value    = channelSettings.wcounter;
    channelEditor.querySelector('[name="burstCount"]').value   = channelSettings.bcounter;
    channelEditor.querySelector('[name="onCount"]').value      = channelSettings.pcounter;
    channelEditor.querySelector('[name="offCount"]').value     = channelSettings.ocounter;
    channelEditor.querySelector('[name="sync"]').value         = channelSettings.sync;
    channelEditor.querySelector('[name="gateMode"]').value     = channelSettings.gate_mode;
    channelEditor.querySelector('[name="gateLogic"]').value    = channelSettings.gate_level;

    const channelStates = channelSettings.states;
    const channelDelays = channelSettings.delays;
    const channelUnits = channelSettings.units;

    for (let i = 0; i < channelInstructionCount; i++) {
        channelEditor.querySelector(`[name="state${i+1}"]`).value     = channelStates[i] ?? 'off';
        channelEditor.querySelector(`[name="delay${i+1}"]`).value     = channelDelays[i] ?? 0.0;
        channelEditor.querySelector(`[name="delay${i+1}Unit"]`).value = channelUnits[i] ?? 's'
    }
}

function updateSystemPeriodMinimum() {
    const periodInput = systemEditor.querySelector('[name="period"]');
    const units = systemEditor.querySelector('[name="periodUnit"]').value;
    const divider = systemEditor.querySelector('[name="divider"]').valueAsNumber;

    periodInput.min = SYSTEM_PERIOD_MIN[units] * divider;
}

function populateSystem() {
    const settings = user.systemSettings;

    systemEditor.querySelector('[name="enable"]').checked       = settings.enable;
    systemEditor.querySelector('[name="divider"]').value        = settings.divider;
    systemEditor.querySelector('[name="period"]').value         = settings.period;
    systemEditor.querySelector('[name="periodUnit"]').value     = settings.units;
    systemEditor.querySelector('[name="burstCounts"]').value    = settings.bcounter;
    systemEditor.querySelector('[name="onCounts"]').value       = settings.pcounter;
    systemEditor.querySelector('[name="offCounts"]').value      = settings.ocounter;
    systemEditor.querySelector('[name="triggerMode"]').value    = settings.trigger_mode;
    systemEditor.querySelector('[name="triggerEdge"]').value    = settings.trigger_edge;
    systemEditor.querySelector('[name="gateMode"]').value       = settings.gate_mode;
    systemEditor.querySelector('[name="gateLogic"]').value      = settings.gate_level;

    updateSystemPeriodMinimum();
}

async function updateChannels(channel = '') {
    if (!channel) {
        const msg = 'Channel name must be specified';
        throw new Error(msg);
    }

    const channelIndex = mapChannelToIndex(channel);
    const settings = user.channelSettings[channelIndex];

    settings.enable     = channelEditor.querySelector('[name="enable"]').checked;
    settings.mode       = channelEditor.querySelector('[name="mode"]').value;
    settings.divider    = channelEditor.querySelector('[name="divider"]').valueAsNumber;
    settings.wcounter   = channelEditor.querySelector('[name="waitCount"]').valueAsNumber;
    settings.bcounter   = channelEditor.querySelector('[name="burstCount"]').valueAsNumber;
    settings.pcounter   = channelEditor.querySelector('[name="onCount"]').valueAsNumber;
    settings.ocounter   = channelEditor.querySelector('[name="offCount"]').valueAsNumber;
    settings.sync       = channelEditor.querySelector('[name="sync"]').value;
    settings.gate_mode  = channelEditor.querySelector('[name="gateMode"]').value;
    settings.gate_level = channelEditor.querySelector('[name="gateLogic"]').value;

    const states = [];
    const delays = [];
    const units = [];

    for (let i = 0; i < channelInstructionCount; i++) {
        states.push(
            channelEditor.querySelector(`[name="state${i + 1}"]`).value ?? 'off'
        );

        delays.push(
            channelEditor.querySelector(`[name="delay${i + 1}"]`).valueAsNumber ?? 0.0
        );

        units.push(
            channelEditor.querySelector(`[name="delay${i + 1}Unit"]`).value ?? 's'
        );
    }

    settings.states = states;
    settings.delays = delays;
    settings.units = units;

    await user.save();
}

async function updateSystem() {
    const settings = user.systemSettings;

    settings.enable     = systemEditor.querySelector('[name="enable"]').checked;
    settings.divider    = systemEditor.querySelector('[name="divider"]').valueAsNumber;
    settings.period     = systemEditor.querySelector('[name="period"]').valueAsNumber;
    settings.units      = systemEditor.querySelector('[name="periodUnit"]').value;
    settings.bcounter   = systemEditor.querySelector('[name="burstCounts"]').valueAsNumber;
    settings.pcounter   = systemEditor.querySelector('[name="onCounts"]').valueAsNumber;
    settings.ocounter   = systemEditor.querySelector('[name="offCounts"]').valueAsNumber;
    settings.trigger_mode   = systemEditor.querySelector('[name="triggerMode"]').value;
    settings.trigger_edge   = systemEditor.querySelector('[name="triggerEdge"]').value;
    settings.gate_mode  = systemEditor.querySelector('[name="gateMode"]').value;
    settings.gate_level = systemEditor.querySelector('[name="gateLogic"]').value;

    await user.save();
}

function resetAll() {
    const channels = [];

    for (let index = 0; index < user.channelSettings.length; index++) {
        channels.push(createChannelSettings());
    }

    user.explicit = {
        systemSettings: createSystemSettings(),
        channelSettings: channels
    };

    populateSystem();
    populateChannels(channelEditor.dataset.channel);
}

function resetSystem() {
    user.systemSettings = createSystemSettings();
    populateSystem();
}

function resetChannel() {
    const channel = channelEditor.dataset.channel;
    const index = mapChannelToIndex(channel);

    const channels = user.channelSettings.slice();
    channels[index] = createChannelSettings();

    user.channelSettings = channels;
    populateChannels(channel);
}

async function onSettingsInput(event) {
    let editor;

    if (systemEditor.contains(event.target)) {
        editor = systemEditor;
        updateSystemPeriodMinimum();
    } else if (channelEditor.contains(event.target)) {
        editor = channelEditor;
    } else {
        return;
    }

    // Wait until all number fields in this section are valid.
    for (const input of editor.querySelectorAll('input[type="number"]')) {
        if (!Number.isFinite(input.valueAsNumber) || !input.checkValidity()) {
            console.warn('Invalid values detected; skipping save')
            
            const msg = 'Invalid values detected; skipping save';
            await showMessage(msg, PARAMETER_ERROR);

            return;
        }
    }

    try {
        if (editor === systemEditor) {
            await updateSystem();
        } else {
            await updateChannels(channelEditor.dataset.channel);
        }

        console.log('saved');
    } catch (error) {
        // And perhaps a modal here too?
        console.error('Could not save settings:', error);
    }
}

function onClickButton(event) {
    const button = event.target.closest("button[data-channel]");
    if (!button) {
        return;
    }

    const channel = button.dataset.channel;

    for (const item of channelList.querySelectorAll("button[data-channel]")) {
        item.setAttribute("aria-pressed", String(item === button));
    }

    channelEditor.dataset.channel = channel;
    channelHeading.textContent = `CH ${channel} settings`;

    populateChannels(channel);
}

function onChangeInput(event) {
    const input = event.target;

    if (
        input.matches('input[type="number"][min="0"]') &&
        input.valueAsNumber < 0
    ) {
        input.value = "0";
    }
}


terminal.addEventListener('terminal-submit', sendTerminalCommand);
channelList.addEventListener("click", onClickButton);
channelEditor.addEventListener("input", onChangeInput);
connectButton.addEventListener('click', manageConnection);
runStopButton.addEventListener('click', manageRunStop);
sendButton.addEventListener('click', sendSettingsToDevice);
document.addEventListener('change', onSettingsInput);
document.querySelector("#reset-all").addEventListener("click", resetAll);
document.querySelector("#reset-system").addEventListener("click", resetSystem);
document.querySelector("#reset-channel").addEventListener("click", resetChannel);

// Prevent scroll wheel from modifying anything
for (const input of document.querySelectorAll('input[type="number"]')) {
    input.addEventListener('wheel',
        function preventWheelChange(event) {
            event.preventDefault();
        },
        { 
            passive: false
        }
    );
}