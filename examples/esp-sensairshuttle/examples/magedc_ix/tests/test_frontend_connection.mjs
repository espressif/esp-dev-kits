// SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
// SPDX-License-Identifier: Apache-2.0

import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { test } from 'node:test';
import vm from 'node:vm';

const html = readFileSync(new URL('../upperlevelmachine/game_demo.html', import.meta.url), 'utf8');
const names = [
    'normalizeHostInput', 'isValidIpv4', 'extractIpv4FromHost',
    'getDeviceHttpBase', 'normalizeGestureSeq', 'requestSpeakerEnabled',
    'requestSpeakerGestureSnapshot', 'requestSpeakerGestureSet', 'requestAudioSnapshot',
    'tryAutoConnectCandidateList', 'connectNetworkHost', 'validateWifiCredentials', 'provisionWifi',
    'initGestureComposerUi',
];
const source = names.map((name) => {
    const match = html.match(new RegExp('^    (?:async )?function ' + name + '\\([^]*?^    }', 'm'));
    assert.ok(match, 'Missing HTML function: ' + name);
    return match[0];
}).join('\n');

function createPage() {
    const events = [];
    const requests = [];
    const statuses = [];
    const timers = new Map();
    const storage = new Map();
    const button = () => ({ textContent: '', classList: { add() {}, remove() {} } });
    let timerId = 0;
    const page = {
        TextEncoder,
        sse: null,
        currentNetworkHost: '',
        networkConnectionId: 0,
        wifiRetryTimer: null,
        connMode: 'none',
        serialWriter: null,
        autoConnectedIp: '',
        detectedDeviceIp: '',
        NETWORK_HOST_STORAGE_KEY: 'host',
        NETWORK_IP_STORAGE_KEY: 'ip',
        NETWORK_SSID_STORAGE_KEY: 'ssid',
        SPEAKER_GESTURE_MAX_LEN: Number(html.match(/const SPEAKER_GESTURE_MAX_LEN = (\d+);/)[1]),
        inputIpEl: { value: '' },
        inputSsidEl: { value: '', focus() {} },
        inputPassEl: { value: '' },
        btnIpConnect: button(),
        debugEl: { innerText: '' },
        document: {
            getElementById: () => ({ innerText: '' }),
            createElement: () => ({ dataset: {}, addEventListener(type, fn) { this[type] = fn; } }),
        },
        localStorage: { getItem: (key) => storage.get(key), setItem: (key, value) => storage.set(key, value) },
        setTimeout: (fn, delay) => { timers.set(++timerId, { fn, delay }); return timerId; },
        clearTimeout: (id) => timers.delete(id),
        EventSource: class {
            constructor(url) { this.url = url; this.closed = false; events.push(this); }
            close() { this.closed = true; }
        },
        fetch: async (url, options) => {
            requests.push({ url, options });
            return { ok: true, status: 200, json: async () => ({ ok: true }) };
        },
        setStatus: (text, kind) => statuses.push({ text, kind }),
        setGestureStatus: (text, kind) => statuses.push({ text, kind }),
        setAutoModeStatus() {},
        setConnectHelp() {},
        startSerialIpProbe() {},
        alert() { assert.fail('Unexpected alert'); },
        parsedLines: [],
        parseLine(text) { page.parsedLines.push(text); },
        speakerState: { known: false, enabled: false },
        customGestureState: { firmwareSpeakerSeq: [1, 9], draftSeq: [], maps: [] },
        CUSTOM_GESTURE_MAP_KEY: 'gestures',
        gestureGridEl: { children: [], appendChild(child) { this.children.push(child); } },
        btnGestureClearEl: null,
        btnGestureSaveEl: null,
        renderGestureGrid() {},
        renderGestureList() {},
    };
    vm.createContext(page);
    vm.runInContext(source, page);
    return {
        page, events, requests, statuses, storage, timers,
        runTimer(delay) {
            const found = [...timers].find(([, timer]) => timer.delay === delay);
            assert.ok(found, 'Expected timer at ' + delay + ' ms');
            timers.delete(found[0]);
            found[1].fn();
        },
    };
}

const settle = async () => { for (let i = 0; i < 6; i++) await Promise.resolve(); };

test('complete inline scripts parse', () => {
    for (const [, code] of html.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/g)) {
        if (code.trim()) new vm.Script(code);
    }
});

test('automatic connection reconnects after success and repeated outages', async () => {
    const h = createPage();
    h.page.tryAutoConnectCandidateList(['192.168.1.10', 'magedc.local']);
    const first = h.events[0];
    first.onopen();
    await settle();
    assert.equal(h.page.connMode, 'net');
    first.onerror();
    assert.equal(h.page.connMode, 'none');
    assert.equal(first.closed, true);
    h.runTimer(3000);
    assert.equal(h.events[1].url, first.url);
    h.events[1].onerror();
    h.runTimer(3000);
    h.events[2].onopen();
    assert.equal(h.page.connMode, 'net');
    assert.equal(h.events[2].url, first.url);
});

test('initial automatic failure tries the next candidate', () => {
    const h = createPage();
    h.page.tryAutoConnectCandidateList(['192.168.1.10', 'magedc.local']);
    h.events[0].onerror();
    h.runTimer(180);
    assert.equal(h.events[1].url, 'http://magedc.local/events');
});

test('manual host change owns SSE, POST routing and retry callbacks', async () => {
    const h = createPage();
    h.page.connectNetworkHost('192.168.1.10');
    const first = h.events[0];
    first.onopen();
    await settle();
    first.onerror();
    const staleRetry = [...h.timers.values()][0].fn;
    h.page.connectNetworkHost('http://magedc.local:8080/events');
    const second = h.events[1];
    assert.equal(await h.page.requestSpeakerEnabled(true), false);
    second.onopen();
    await settle();
    h.requests.length = 0;
    await h.page.requestSpeakerEnabled(true);
    await h.page.requestSpeakerGestureSet([2, 8]);
    await h.page.requestAudioSnapshot();
    await settle();
    assert.ok(h.requests.length >= 4);
    assert.ok(h.requests.every((request) => request.url === 'http://magedc.local:8080/api/audio'));
    first.onopen();
    first.onmessage({ data: 'stale' });
    first.onerror();
    staleRetry();
    assert.equal(h.page.sse, second);
    assert.equal(h.page.currentNetworkHost, 'magedc.local:8080');
    assert.equal(h.page.connMode, 'net');
    assert.equal(h.events.length, 2);
    assert.deepEqual(h.page.parsedLines, []);
    second.onmessage({ data: 'current' });
    assert.deepEqual(h.page.parsedLines, ['current']);
});

test('gesture save rejects nine steps and requires explicit backend success', async () => {
    const h = createPage();
    h.page.connMode = 'net';
    h.page.currentNetworkHost = 'magedc.local';
    assert.equal(await h.page.requestSpeakerGestureSet([1, 2, 3, 4, 5, 6, 7, 8, 9]), false);
    assert.equal(h.requests.length, 0);
    for (const response of [
        { ok: false, status: 400, result: { ok: false, err: 'invalid_command' } },
        { ok: false, status: 500, result: { ok: false, err: 'save_failed' } },
        { ok: true, status: 200, result: { ok: false, err: 'save_failed' } },
        { ok: true, status: 200, result: {} },
    ]) {
        h.page.fetch = async () => ({ ...response, json: async () => response.result });
        assert.equal(await h.page.requestSpeakerGestureSet([2, 8]), false);
        assert.deepEqual(Array.from(h.page.customGestureState.firmwareSpeakerSeq), [1, 9]);
    }
    h.page.fetch = async () => ({ ok: true, status: 200, json: async () => ({ ok: true }) });
    assert.equal(await h.page.requestSpeakerGestureSet([1, 2, 3, 4, 5, 6, 7, 8]), true);
    assert.equal(h.page.customGestureState.firmwareSpeakerSeq.length, 8);
    assert.match(h.statuses.at(-1).text, /已写入设备 NVS/);
});

test('gesture composer stops at eight steps without silently truncating saved sequences', () => {
    const h = createPage();
    h.page.initGestureComposerUi();
    for (const button of h.page.gestureGridEl.children) button.click();
    assert.deepEqual(Array.from(h.page.customGestureState.draftSeq), [1, 2, 3, 4, 5, 6, 7, 8]);
    assert.match(h.statuses.at(-1).text, /最多 8 步/);
});

test('late save response from the previous device cannot update the new device', async () => {
    const h = createPage();
    h.page.connMode = 'net';
    h.page.currentNetworkHost = 'device-a.local';
    let finish;
    h.page.fetch = () => new Promise((resolve) => { finish = resolve; });
    const save = h.page.requestSpeakerGestureSet([2, 8]);
    h.page.currentNetworkHost = 'device-b.local';
    finish({ ok: true, status: 200, json: async () => ({ ok: true }) });
    assert.equal(await save, false);
    assert.deepEqual(Array.from(h.page.customGestureState.firmwareSpeakerSeq), [1, 9]);
});

test('credentials use UTF-8 byte limits and preserve leading/trailing spaces', async () => {
    const h = createPage();
    for (const [ssid, pass] of [
        ['a'.repeat(32), ''], ['中'.repeat(10), '中'.repeat(21)],
        [' ssid ', ' 123456 '], [' ', ' '.repeat(8)], ['ssid', 'a'.repeat(64)],
    ]) assert.equal(h.page.validateWifiCredentials(ssid, pass), '');
    for (const [ssid, pass] of [
        ['', ''], ['a'.repeat(33), ''], ['中'.repeat(11), ''],
        ['ssid', '1234567'], ['ssid', '中'.repeat(22)], ['ssid', 'g'.repeat(64)],
        ['a\tb', ''], ['ssid', '1234567\n'], ['a\0b', ''],
    ]) assert.notEqual(h.page.validateWifiCredentials(ssid, pass), '');
    const writes = [];
    h.page.serialWriter = { write: async (text) => writes.push(text) };
    h.page.inputSsidEl.value = ' ssid ';
    h.page.inputPassEl.value = ' 123456 ';
    await h.page.provisionWifi();
    assert.deepEqual(writes, ['WIFI_CFG\t ssid \t 123456 \n']);
    assert.equal(h.storage.get('ssid'), ' ssid ');
    h.page.inputSsidEl.value = '中'.repeat(11);
    await h.page.provisionWifi();
    assert.equal(writes.length, 1);
});
