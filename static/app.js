(() => {
'use strict';

// ── Uptime counter ──────────────────────────────────────────────────────────
const start = Date.now();
const uptimeEl = document.getElementById('stat-uptime');
const footerEl = document.getElementById('footer-uptime');

function fmtUptime(ms) {
    const s = Math.floor(ms / 1000);
    if (s < 60)  return s + 's';
    const m = Math.floor(s / 60), rs = s % 60;
    if (m < 60)  return m + 'm ' + rs + 's';
    const h = Math.floor(m / 60), rm = m % 60;
    return h + 'h ' + rm + 'm';
}

setInterval(() => {
    const t = fmtUptime(Date.now() - start);
    if (uptimeEl) uptimeEl.textContent = t;
    if (footerEl) footerEl.textContent = 'Page uptime: ' + t;
}, 1000);

// ── Fake request counter ────────────────────────────────────────────────────
let reqCount = 0;
const reqEl = document.getElementById('stat-req');
function bumpReq(n = 1) {
    reqCount += n;
    if (reqEl) reqEl.textContent = reqCount;
}

// ── Request Simulator ───────────────────────────────────────────────────────
const simBtn    = document.getElementById('sim-btn');
const simMethod = document.getElementById('sim-method');
const simPath   = document.getElementById('sim-path');
const simOut    = document.getElementById('sim-output');

const MIME_MAP = {
    '.html': 'text/html',
    '.htm':  'text/html',
    '.css':  'text/css',
    '.js':   'application/javascript',
    '.json': 'application/json',
    '.png':  'image/png',
    '.jpg':  'image/jpeg',
    '.jpeg': 'image/jpeg',
    '.gif':  'image/gif',
    '.svg':  'image/svg+xml',
    '.ico':  'image/x-icon',
    '.txt':  'text/plain',
};

const STATIC_FILES = ['/index.html', '/style.css', '/app.js'];

function getMime(path) {
    const dot = path.lastIndexOf('.');
    if (dot === -1) return 'application/octet-stream';
    return MIME_MAP[path.slice(dot)] || 'application/octet-stream';
}

function isSafe(path) {
    return !path.includes('..');
}

function simulateResponse(method, path) {
    // Normalise
    if (path === '/' || path === '') path = '/index.html';
    const stripQ = path.split('?')[0];

    if (!isSafe(stripQ)) {
        return { status: 403, text: 'Forbidden', mime: 'text/html',
                 body: '<h1>403 Forbidden</h1>' };
    }

    if (method !== 'GET' && method !== 'HEAD') {
        return { status: 405, text: 'Method Not Allowed', mime: 'text/html',
                 body: '<h1>405 Method Not Allowed</h1>' };
    }

    if (!STATIC_FILES.includes(stripQ)) {
        return { status: 404, text: 'Not Found', mime: 'text/html',
                 body: '<h1>404 Not Found</h1><p>' + stripQ + '</p>' };
    }

    const mime = getMime(stripQ);
    const body = method === 'HEAD' ? '' : '(binary / file content)';
    return { status: 200, text: 'OK', mime, body };
}

function colorLine(text, cls) {
    const span = document.createElement('span');
    span.className = cls;
    span.textContent = text;
    return span;
}

function renderSim(method, path) {
    const r = simulateResponse(method, path);
    simOut.innerHTML = '';

    const statusCls = r.status === 200 ? 'sim-line-status'
                    : r.status === 405 ? 'sim-line-warn'
                    : 'sim-line-error';

    const lines = [
        { text: '> ' + method + ' ' + path + ' HTTP/1.1',          cls: 'sim-line-header' },
        { text: '> Host: localhost:8080',                            cls: 'sim-line-header' },
        { text: '',                                                  cls: '' },
        { text: '< HTTP/1.1 ' + r.status + ' ' + r.text,           cls: statusCls },
        { text: '< Content-Type: ' + r.mime,                        cls: 'sim-line-header' },
        { text: '< Content-Length: ' + r.body.length,               cls: 'sim-line-header' },
        { text: '< Connection: close',                               cls: 'sim-line-header' },
        { text: '',                                                  cls: '' },
        { text: r.body || '(no body — HEAD request)',               cls: 'sim-line-body' },
    ];

    lines.forEach(l => {
        if (l.text === '') {
            simOut.appendChild(document.createTextNode('\n'));
            return;
        }
        const span = colorLine(l.text, l.cls);
        simOut.appendChild(span);
        simOut.appendChild(document.createTextNode('\n'));
    });

    bumpReq();
    animateThread();
}

if (simBtn) {
    simBtn.addEventListener('click', () => {
        renderSim(simMethod.value, simPath.value.trim() || '/');
    });
}

// ── Thread pool visualiser ──────────────────────────────────────────────────
const THREAD_COUNT = 4;
let threadBusy = Array(THREAD_COUNT).fill(false);
let threadQueue = 0;

function setThreadState(id, state) {
    const bar    = document.querySelector(`.pool-lane[data-id="${id}"] .pool-bar`);
    const status = document.getElementById('ts-' + id);
    if (!bar || !status) return;

    bar.className = 'pool-bar ' + state;
    status.className = 'pool-status' + (state === 'active' ? ' status-active' : state === 'done' ? ' status-done' : '');
    status.textContent = state === 'active' ? 'busy' : state === 'done' ? 'done' : 'idle';
}

function animateThread(count = 1) {
    for (let i = 0; i < count; i++) {
        // Find an idle thread or queue
        const idle = threadBusy.findIndex(b => !b);
        if (idle === -1) { threadQueue++; continue; }

        threadBusy[idle] = true;
        setThreadState(idle, 'active');

        const duration = 400 + Math.random() * 600;
        setTimeout(() => {
            setThreadState(idle, 'done');
            setTimeout(() => {
                setThreadState(idle, 'idle');
                threadBusy[idle] = false;

                if (threadQueue > 0) {
                    threadQueue--;
                    animateThread(1);
                }
            }, 350);
        }, duration);
    }
}

// Burst button
const burstBtn = document.getElementById('burst-btn');
if (burstBtn) {
    burstBtn.addEventListener('click', () => {
        const count = 4 + Math.floor(Math.random() * 5);
        bumpReq(count);
        animateThread(count);
    });
}

// ── Copy buttons ────────────────────────────────────────────────────────────
document.querySelectorAll('.copy-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        const target = document.getElementById(btn.dataset.target);
        if (!target) return;
        navigator.clipboard.writeText(target.textContent.trim()).then(() => {
            btn.textContent = '✓';
            btn.classList.add('copied');
            setTimeout(() => {
                btn.textContent = '⧉';
                btn.classList.remove('copied');
            }, 1500);
        });
    });
});

// ── Idle thread activity pulse (ambient) ────────────────────────────────────
setInterval(() => {
    if (Math.random() < 0.2) {
        bumpReq(1);
        animateThread(1);
    }
}, 3000);

})();
