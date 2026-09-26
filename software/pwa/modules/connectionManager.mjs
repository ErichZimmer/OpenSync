// OpenSync currently uses RPI vendor descriptors
const OPENSYNC_USB = {
    usbVendorId: 0x2E8A,
    usbProductId: 0x000A
};

async function findDevice() {
    return await navigator.serial.requestPort(
        {
            filters: [OPENSYNC_USB]
        }
    );
}

async function openDevice(port) {
    return await port.open(
        {
            baudRate: 19200
        }
    );
}

async function closeDevice(port) {
    return await port.close();
}

async function writeSCPI(port, command) {
    const writer = port.writable.getWriter();

    try {
        // :SYST:ERR? forces a serial output for all commands.
        // This is used to prevent hangs when reading serial output and there is no output.
        const data = new TextEncoder().encode(command + ';:SYST:ERR?\r\n');
        await writer.write(data);
    } finally {
        writer.releaseLock();
    }
}

async function readSCPI(port) {
    const reader = port.readable.getReader();
    const decoder = new TextDecoder();
    let response = '';

    try {
        while (!response.includes('\n')) {
            const { value, done } = await reader.read();

            if (done) {
                break;
            }

            response += decoder.decode(value, { stream: true });
        }

        response = response.trim();

        // remove success reports, we are only concerned with errors
        response = response.replace('0,"No error"', '')

        // Remove any trailing ';'
        response = response.replace(';', '');

        return response;
    } finally {
        reader.releaseLock();
    }
}

async function sendCommand(port, command) {
    await writeSCPI(port, command);

    return await readSCPI(port);
}


class DeviceManager {
    #device = undefined;

    constructor() {
        navigator.serial.addEventListener('disconnect',
            function onDisconnect(event) {
                if (event.target === this.#device) {
                    this.#device = undefined;
                }
            }.bind(this)
        );
    }

    async open() {
        try {
            this.#device = await findDevice();
            await openDevice(this.#device);

            return true;
        }
        catch (err) {
            const msg = `Unable to establish OpenSync connection. Reason: ${err.message}`;
            throw new Error(msg);
        }
    }

    isopen() {
        if (!this.#device) {
            return false;
        }

        return true;
    }

    async send(command='') {
        if (!command) {
            return false;
        }

        if (!this.#device) {
            const msg = 'OpenSync device has not established a connection.'
            throw new Error(msg);
        }

        return await sendCommand(this.#device, command);
    }

    async close() {
        if (this.#device) {
            await closeDevice(this.#device);
            this.#device = undefined;
        }

        return true;
    }
}


export default DeviceManager;