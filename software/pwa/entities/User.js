import Is from 'strong-type';
import {
    arcaneEvents,
    createArcaneEventSource,
    projectArcaneDOMEvent
} from 'arcane-os/event-manager';
import { arcaneLogging } from 'arcane-os/logging';

// Initializes window.dbopfs.
import DBOPFS from 'arcane-os/modules/DBOPFS.js';

const is = new Is(false);
let singletonDBOPFSReadyUnsubscribe = null;

// Minimum clock period at divider 1, expressed in each selectable unit.
const SYSTEM_PERIOD_MIN = {
    s: 0.00000032,
    ms: 0.00032,
    us: 0.32,
    ns: 320
};

function createSystemSettings() {
    return {
        enable: false,
        divider: 1,
        period: 0.1,
        units: 's',
        bcounter: 0,
        pcounter: 1,
        ocounter: 0,
        trigger_mode: 'disabled',
        trigger_edge: 'rising',
        gate_mode: 'disabled',
        gate_level: 'high'
    };
}

function createChannelSettings() {
    return {
        enable: false,
        mode: 'lvttl',
        divider: 1,
        sync: 'T0',
        wcounter: 0,
        bcounter: 0,
        pcounter: 1,
        ocounter: 0,
        gate_mode: 'disabled',
        gate_level: 'high',
        states: [],
        delays: [],
        units: []
    };
}

function createChannels() {
    const channels = [];

    for (let index = 0; index < 8; index++) {
        channels.push(createChannelSettings());
    }

    return channels;
}

// Copy declared settings into a new default object.
function readSettings(value, defaults, path) {
    if (!value || !is.object(value) || is.array(value)) {
        const msg = path + ' must be an object';
        throw new TypeError(msg);
    }

    for (const key in value) {
        if (!Object.hasOwn(value, key)) {
            continue;
        }

        if (!Object.hasOwn(defaults, key)) {
            const msg = path + ' has an unknown setting: ' + key;
            throw new TypeError(msg);
        }
    }

    for (const key in defaults) {
        if (Object.hasOwn(value, key)) {
            defaults[key] = value[key];
        }
    }
    return defaults;
}

function validateInteger(value, minimum, maximum, path) {
    if (!is.integer(value) || value < minimum || value > maximum) {
        const msg = path + ' must be an integer from ' + minimum + ' to ' + maximum;
        throw new TypeError(msg);
    }
}

function validateChoice(value, choices, path) {
    if (!choices.includes(value)) {
        const msg = path + ' must be one of: ' + choices.join(', ');
        throw new TypeError(msg);
    }
}

function validateCommonSettings(settings, path) {
    if (!is.boolean(settings.enable)) {
        const msg = path + '.enable must be a boolean';
        throw new TypeError(msg);
    }

    validateInteger(settings.divider, 1, 65500, path + '.divider');
    validateInteger(settings.bcounter, 0, 30000000, path + '.bcounter');
    validateInteger(settings.pcounter, 1, 1000000000, path + '.pcounter');
    validateInteger(settings.ocounter, 0, 1000000000, path + '.ocounter');
    validateChoice(settings.gate_level, ['high', 'low'], path + '.gate_level');
}

function validateSystemSettings(value) {
    const settings = readSettings(value, createSystemSettings(), 'systemSettings');
    validateCommonSettings(settings, 'systemSettings');

    validateChoice(settings.units, ['s', 'ms', 'us', 'ns'], 'systemSettings.units');

    // Store the period value with its selected unit; do not convert it here.
    const minimumPeriod = SYSTEM_PERIOD_MIN[settings.units] * settings.divider;
    if (!is.finite(settings.period) || settings.period < minimumPeriod) {
        throw new TypeError('systemSettings.period must be at least ' + minimumPeriod + ' ' + settings.units);
    }

    validateChoice(settings.trigger_mode, ['disabled', 'triggered'], 'systemSettings.trigger_mode');
    validateChoice(settings.trigger_edge, ['rising', 'falling'], 'systemSettings.trigger_edge');
    validateChoice(settings.gate_mode, ['disabled', 'pulse', 'output', 'channel'], 'systemSettings.gate_mode');
    
    return settings;
}

function validateChannelSettings(value, index) {
    const path = 'channelSettings[' + index + ']';
    const settings = readSettings(value, createChannelSettings(), path);
    
    validateCommonSettings(settings, path);
    validateChoice(settings.mode, ['lvttl', 'ttl'], path + '.mode');
    validateInteger(settings.wcounter, 0, 1000000000, path + '.wcounter');
    validateChoice(settings.sync, ['T0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'], path + '.sync');
    validateChoice(settings.gate_mode, ['disabled', 'pulse', 'output'], path + '.gate_mode');

    if (!is.array(settings.states) || !is.array(settings.delays)) {
        throw new TypeError(path + '.states and .delays must be arrays');
    }

    if (settings.states.length !== settings.delays.length) {
        throw new TypeError(path + '.states and .delays must have matching lengths');
    }

    // Saved channels from before units were added contain delays in seconds.
    if (!Object.hasOwn(value, 'units')) {
        for (let step = 0; step < settings.delays.length; step++) {
            settings.units.push('s');
        }
    }

    if (!is.array(settings.units)) {
        throw new TypeError(path + '.units must be an array');
    }

    if (settings.units.length !== settings.delays.length) {
        throw new TypeError(path + '.units must contain one unit for each delay');
    }

    const states = [];
    const delays = [];
    const units = [];

    for (let step = 0; step < settings.states.length; step++) {
        validateChoice(settings.states[step], ['on', 'off'], path + '.states[' + step + ']');
        
        if (!is.finite(settings.delays[step]) || settings.delays[step] < 0) {
            const msg = path + '.delays[' + step + '] must be a nonnegative number';
            throw new TypeError(msg);
        }

        validateChoice(settings.units[step], ['s', 'ms', 'us', 'ns'], path + '.units[' + step + ']');

        states.push(settings.states[step]);
        delays.push(settings.delays[step]);
        units.push(settings.units[step]);
    }

    settings.states = states;
    settings.delays = delays;
    settings.units = units;

    return settings;
}

function validateChannels(value) {
    if (!is.array(value) || value.length !== 8) {
        const msg = 'channelSettings must contain exactly 8 channels';
        throw new TypeError(msg);
    }

    const channels = [];

    for (let index = 0; index < 8; index++) {
        channels.push(validateChannelSettings(value[index], index));
    }

    return channels;
}

class UserEntity {
    #events;
    #disposed = false;
    #loadPromise = null;
    #stopDBOPFSReady = null;
    #tableName = 'users';
    #schema = ['systemSettings', 'channelSettings'];
    #systemSettings = createSystemSettings();
    #channelSettings = createChannels();

    fileName = 'users.json';
    persist = true;
    ready = false;

    constructor(fileName = '') {
        if (window.user) {
            return window.user;
        }
        if (is.string(fileName) && fileName) {
            this.fileName = fileName;
        }

        this.#events = createArcaneEventSource(this,
            {
                source: 'user-entity',
                eventTypes: ['user-entity-loaded']
            }
        );

        if (window.dbopfs?.ready) {
            this.#loadAutomatically();
        } else {
            this.#stopDBOPFSReady = arcaneEvents.subscribe(
                'dbopfs-ready',
                function stopDBOPFSReadyFunc() {
                    this.#stopDBOPFSReady = null;
                    this.#loadAutomatically();
                }.bind(this),
                { once: true }
            );
        }
    }

    // These getters return the actual settings, just like the SDK User entity.
    // After a nested edit, call save(). Replacing a whole setting uses its setter.
    get systemSettings() {
        return this.#systemSettings;
    }

    set systemSettings(value) {
        this.#requireReady();
        this.#systemSettings = validateSystemSettings(value);
        this.#persist();
    }

    get channelSettings() {
        return this.#channelSettings;
    }

    set channelSettings(value) {
        this.#requireReady();
        this.#channelSettings = validateChannels(value);
        this.#persist();
    }

    // Validate and copy the two declared fields before serialization.
    get explicit() {
        return {
            systemSettings: validateSystemSettings(this.#systemSettings),
            channelSettings: validateChannels(this.#channelSettings)
        };
    }

    set explicit(value) {
        this.#requireActive();
        if (is.string(value)) {
            value = JSON.parse(value);
        }

        if (!value || !is.object(value) || is.array(value)) {
            throw new TypeError('UserEntity.explicit must be an object or JSON object');
        }

        for (const key in value) {
            if (Object.hasOwn(value, key) && !this.#schema.includes(key)) {
                throw new TypeError('UserEntity has an unknown setting: ' + key);
            }
        }

        let system = this.#systemSettings;
        let channels = this.#channelSettings;

        if (Object.hasOwn(value, 'systemSettings')) {
            system = validateSystemSettings(value.systemSettings);
        }

        if (Object.hasOwn(value, 'channelSettings')) {
            channels = validateChannels(value.channelSettings);
        }

        // Apply only after both supplied fields have passed validation.
        this.#systemSettings = system;
        this.#channelSettings = channels;
        this.#persist();
    }

    async load() {
        this.#requireActive();
        if (this.ready) {
            return this.explicit;
        }

        if (!this.#loadPromise) {
            this.#loadPromise = this.#load();
        }

        try {
            return await this.#loadPromise;
        } finally {
            this.#loadPromise = null;
        }
    }

    async #load() {
        await this.#read(false);
        this.ready = true;

        const { occurrence } = this.#events.dispatch(
            'user-entity-loaded',
            { reason: 'user-data-loaded', user: this },
            { publicDetail: { ready: true, reason: 'user-data-loaded' } }
        );

        projectArcaneDOMEvent(window, occurrence);

        return this.explicit;
    }

    async #read(force) {
        this.#requireActive();
        await window.dbopfs.readyPromise;
        const stored = await window.dbopfs.get(this.#tableName, this.fileName, force);
        this.#requireActive();

        if (stored !== null && stored !== undefined) {
            const persist = this.persist;
            this.persist = false;

            try {
                this.explicit = stored;
            } finally {
                this.persist = persist;
            }
        }

        return this.explicit;
    }

    async refresh() {
        await this.load();
        return this.#read(true);
    }

    async save() {
        this.#requireReady();
        const data = this.explicit;
        
        await window.dbopfs.set(this.#tableName, this.fileName, JSON.stringify(data));
        
        return data;
    }

    toJSON() {
        return JSON.stringify(this.explicit);
    }

    #persist() {
        if (this.persist) {
            this.save().catch(
                function saveError(error) {
                    arcaneLogging.error('OpenSync settings could not be saved:', error);
               }   
            );
        }
    }

    #loadAutomatically() {
        this.load().catch(
            function loadError(error) {
                arcaneLogging.error('OpenSync settings could not be loaded:', error);
            }
        );
    }

    #requireActive() {
        if (this.#disposed) {
            const msg = 'UserEntity has been disposed';
            throw new Error(msg);
        }
    }

    #requireReady() {
        this.#requireActive();
        if (!this.ready) {
            const msg = 'Await user.load() before changing or saving settings';
            throw new Error(msg);
        }
    }

    dispose() {
        if (this.#disposed) {
            return false;
        }

        this.#disposed = true;
        this.ready = false;
        this.#stopDBOPFSReady?.();
        this.#stopDBOPFSReady = null;

        if (window.user === this) {
            delete window.user;
            singletonDBOPFSReadyUnsubscribe?.();
            singletonDBOPFSReadyUnsubscribe = null;
        }

        return this.#events.dispose();
    }

    destroy() {
        return this.dispose();
    }
}

function initSingletonUserEntity() {
    singletonDBOPFSReadyUnsubscribe = null;
    if (!window.user) {
        window.user = new UserEntity();
    }
}

if (window.dbopfs?.ready) {
    initSingletonUserEntity();
} else {
    singletonDBOPFSReadyUnsubscribe = arcaneEvents.subscribe(
        'dbopfs-ready', initSingletonUserEntity, { once: true }
    );
}

export { createSystemSettings, createChannelSettings, SYSTEM_PERIOD_MIN };
export default UserEntity;
