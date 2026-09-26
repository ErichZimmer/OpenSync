import {registerPwa, mountPwaInstallPrompt} from "/node_modules/arcane-os/browser-runtime/pwa.mjs";

mountPwaInstallPrompt({appName: "OpenSync Synchronizer"}).catch(
    function reportPwaInstallComponentFailure(error) {
        console.error('Arcane PWA install component failed:', error);
    }
);

const controller = registerPwa(
    {
        workerUrl: new URL('./arcane-sw.js', import.meta.url).href,
        scope: new URL('./', import.meta.url).href
    }
);

controller.ready.catch(
    function reportPwaRegistrationFailure(error) {
        console.error('Arcane PWA registration failed:', error);
    }
);
