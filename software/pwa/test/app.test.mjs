import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import test from 'arcane-os/testing';

const appRoot=new URL('../',import.meta.url);

test('application shell uses the shared Arcane theme in order',async()=>{
    const [source,appSource]=await Promise.all([
        readFile(new URL('index.html',appRoot),'utf8'),
        readFile(new URL('modules/App.js',appRoot),'utf8')
    ]);
    const theme=source.indexOf('./node_modules/arcane-os/runtime/arcane/css/theme.css');
    const primitives=source.indexOf('./node_modules/arcane-os/runtime/arcane/css/primitives.css');
    const appStyle=source.indexOf('./opensync-pwa.css');
    const importMap=source.indexOf('data-arcane-import-map');
    const appModule=source.indexOf('./modules/App.js');

    assert.ok(source.includes('<base href="./">'));
    assert.match(source,/<meta name="arcane-app-id" content="opensync-pwa">/);
    assert.ok(theme>=0&&primitives>theme&&appStyle>primitives);
    assert.ok(importMap>=0&&appModule>importMap);
    assert.ok(appSource.includes("from 'arcane-os/modules/ThemeBootstrap.js'"));
    assert.ok(appSource.includes("from 'arcane-os/app-data-scope'"));
});

test('application package identity matches its directory',async()=>{
    const manifest=JSON.parse(await readFile(new URL('arcane-package.json',appRoot),'utf8'));
    assert.equal(manifest.id,'opensync-pwa');
    assert.equal(manifest.strategy,'static');
    assert.deepEqual(manifest.shared,['browser-runtime']);
});
