# Arcane Opensync Pwa

This repository contains the portable Arcane application `opensync-pwa`. It includes the selected SDK runtime for distribution and has no runtime dependency on an Arcane OS source checkout.

## Start

```sh
npm install
npm run import-map
npm run dev
```

Open the loopback URL printed by the development server. This root app reads SDK files directly from its installed npm package; an ordinary static host uses the same resource paths. The SDK server does not expose an Ollama HTTP endpoint.

Commit the generated `package-lock.json` after dependency installation. CI intentionally uses `npm ci` and therefore requires that lock. The SDK is a runtime dependency: keep it installed when serving this app directly from node_modules. Before the SDK is published, install a locally packed `arcane-os` `.tgz` with `npm install --save-prod --save-exact <path-to-tarball>`; keep that tarball at the lock file's relative path for repeatable local `npm ci` runs.

## Optional browser release commands

```sh
npm run import-map
npm run package
npm run verify
npm run bundle
npm run run
```

The explicit `import-map` command refreshes
`modules/arcane.importmap.json` and the managed inline browser
import map in every directly navigable descriptor-admitted `.html`/`.htm`
document. HTML component fragments remain package files but do not receive a
document-level base or managed import map.
Development, package, and build refresh that shared inventory when the selected operation needs it.
Commit generated import maps and enabled offline app files. Hosting workflows consume those committed files rather than generating them.
Named `arcane-os/modules/* and arcane-os/entities/*` imports resolve through the managed map to the selected SDK files. Packaging copies the complete selected application, runtime, and specifier
map to `dist/opensync-pwa` without running application tests. Run `verify` only when
the user explicitly selects verification or a release artifact that requires it;
`bundle` creates the distributable archive and `run` launches the selected
packaged browser release.

Native targets are provider-supplied and must be scaffolded and selected
explicitly; this browser workflow does not imply a standalone native executable.


Every browser release also carries Arcane OS licensing material under `node_modules/arcane-os`. Review those terms before distribution.
