// Colors
const COLORS = {
    nodeStroke: '#d6ccc7ff',       // nodes outline
    nodeFill: '#848a8aff',       // nodes fill
    route: '#da2d21ff',    // main path (blue)
    shortcut: 'rgba(209, 33, 20, 1)',       // shortcut path (red)
    start: '#2ecc71',    // start marker (green)
    end: '#e74c3c',    // end marker (red)
};

// 1) Init map
const map = L.map('map', {
    preferCanvas: true
}).setView([20, 0], 2);

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution:
        '&copy; <a href="https://www.openstreetmap.org/copyright">OSM</a> contributors'
}).addTo(map);

// Cluster layer for high-volume points
const clusters = L.markerClusterGroup({
    chunkedLoading: true,      // progressive addition for responsiveness
    disableClusteringAtZoom: 14
});
map.addLayer(clusters);

const statusEl = document.getElementById('status');
const fileInput = document.getElementById('file-input');

document.getElementById('toggle-shortcut').addEventListener('change', (e) => {
    if (e.target.checked) shortcutLayer.addTo(map);
    else map.removeLayer(shortcutLayer);
});

// 2) File -> parse -> plot
fileInput.addEventListener('change', async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;

    statusEl.textContent = `Reading "${file.name}"...`;

    const text = await file.text();
    const { nodes, edges } = parseGraphFormatSafe(text);

    plotGraph(nodes, edges);
});

fileInput.addEventListener('change', async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;

    statusEl.textContent = `Reading "${file.name}"...`;
    try {
        const text = await file.text();
        const { nodes, edges } = parseGraphFormatSafe(text);
        plotGraph(nodes, edges);
    } catch (err) {
        console.error(err);
        statusEl.textContent = `Parse error: ${err.message || err}`;
        alert(statusEl.textContent);
    }
});

function parseGraphFormatSafe(txt) {
    // Strip UTF-8 BOM if present
    txt = txt.replace(/^\uFEFF/, '');

    // Normalize newlines and trim
    const rawLines = txt.replace(/\r\n/g, '\n').split('\n');

    // Remove empty lines & comments
    const lines = rawLines
        .map(l => l.trim())
        .filter(l => l.length > 0 && !l.startsWith('#') && !l.startsWith('//'));

    if (lines.length < 2) {
        throw new Error(`Expected at least 2 header lines, got ${lines.length}.`);
    }

    const nNodes = parseInt(lines[0], 10);
    const nEdges = parseInt(lines[1], 10);
    if (!Number.isInteger(nNodes) || !Number.isInteger(nEdges)) {
        throw new Error(`Header must be integers. Got "${lines[0]}", "${lines[1]}".`);
    }

    const needed = 2 + nNodes + nEdges;
    if (lines.length < needed) {
        throw new Error(
            `File too short: header says ${nNodes} nodes + ${nEdges} edges (= ${needed} lines), but only ${lines.length} lines after cleanup.`
        );
    }

    const nodes = [];
    for (let i = 0; i < nNodes; i++) {
        const lnIdx = 2 + i;
        const line = lines[lnIdx];
        if (!line) throw new Error(`Missing node line at index ${lnIdx} (1-based line ${lnIdx + 1}).`);

        const parts = line.split(/\s+/);
        if (parts.length < 3) {
            throw new Error(`Bad node line at ${lnIdx + 1}: "${line}" (expected "id lat lon").`);
        }

        const id = parseInt(parts[0], 10);
        const lat = parseFloat(parts[1]);
        const lon = parseFloat(parts[2]);
        if (!Number.isInteger(id) || !Number.isFinite(lat) || !Number.isFinite(lon)) {
            throw new Error(`Invalid node fields at line ${lnIdx + 1}: "${line}".`);
        }
        nodes.push({ id, lat, lon });
    }

    const edges = [];
    for (let j = 0; j < nEdges; j++) {
        const lnIdx = 2 + nNodes + j;
        const line = lines[lnIdx];
        if (!line) throw new Error(`Missing edge line at index ${lnIdx} (1-based line ${lnIdx + 1}).`);

        const parts = line.split(/\s+/);
        if (parts.length < 2) {
            throw new Error(`Bad edge line at ${lnIdx + 1}: "${line}" (expected "u v").`);
        }

        const u = parseInt(parts[0], 10);
        const v = parseInt(parts[1], 10);
        if (!Number.isInteger(u) || !Number.isInteger(v)) {
            throw new Error(`Invalid edge fields at line ${lnIdx + 1}: "${line}".`);
        }
        edges.push([u, v]);
    }

    // Optional: sanity check node IDs exist
    const nodeIds = new Set(nodes.map(n => n.id));
    for (const [u, v] of edges) {
        if (!nodeIds.has(u) || !nodeIds.has(v)) {
            console.warn('Edge references missing node id:', u, v);
        }
    }

    return { nodes, edges };
}

// route layer to clear/redraw paths
let routeLayer = L.layerGroup().addTo(map);
let shortcutLayer = L.layerGroup().addTo(map);
let edgesLayer = L.layerGroup().addTo(map);

// keep a fast lookup of nodes by id
let nodeById = new Map();

// selection state
let selectedSrc = null;
let selectedDst = null;

// ---- plotting ----
function plotGraph(nodes, edges) {
    //clear layers
    clusters.clearLayers();
    routeLayer.clearLayers();
    //   edgesLayer.clearLayers();
    nodeById = new Map();

    //   const markerById = {};
    const markers = [];

    for (const n of nodes) {
        nodeById.set(n.id, n);

        const marker = L.circleMarker([n.lat, n.lon], {
            radius: 6,
            weight: 1,
            color: COLORS.nodeStroke,
            fillColor: COLORS.nodeFill,
            fillOpacity: 0.9,
        }).bindPopup(`Node ${n.id}<br>Lat: ${n.lat}<br>Lon: ${n.lon}`);

        marker.on('click', () => handleMarkerClick(n.id, marker));
        clusters.addLayer(marker);
        markers.push(marker);
    }

    // draw edges as lines
    for (const [u, v] of edges) {
        const nu = nodeById.get(u), nv = nodeById.get(v);
        if (nu && nv) L.polyline([[nu.lat, nu.lon], [nv.lat, nv.lon]], { weight: 2, opacity: 0.6 }).addTo(edgesLayer);
    }

    if (markers.length) {
        const group = L.featureGroup(markers);
        map.fitBounds(group.getBounds().pad(0.2));
        statusEl.textContent = `Loaded ${nodes.length} nodes and ${edges.length} edges.`;
    } else {
        statusEl.textContent = 'No valid nodes found.';
    }
}

let highlightedMarker = null;

// Click Locations
function handleMarkerClick(nodeId, marker) {
    if (selectedSrc === null) {
        selectedSrc = nodeId;

        // reset old highlighted marker
        if (highlightedMarker) {
            highlightedMarker.setStyle({
                radius: 4,
                color: COLORS.nodeStroke,
                fillColor: COLORS.nodeFill,
            });
        }

        // highlight this marker
        highlightedMarker = marker;
        marker.setStyle({
            radius: 7,
            color: COLORS.route,
            fillColor: COLORS.route,
        });

        statusEl.textContent = `Source = ${nodeId}. Now click a destination node.`;
        return;
    }
    if (selectedDst === null) {
        selectedDst = nodeId;
        statusEl.textContent = `Computing route ${selectedSrc} → ${selectedDst}...`;
        requestRoute(selectedSrc, selectedDst)
            .then(drawRoute)
            .catch(err => {
                console.error(err);
                statusEl.textContent = `Route failed: ${err?.message || err}`;
            })
            .finally(() => {
                selectedSrc = null;
                selectedDst = null;
                if (highlightedMarker) highlightedMarker.setStyle({ radius: 5 });
                highlightedMarker = null;
            });
    }
}

// Update the Graph Name in order to use some other map
let GRAPH_NAME = 'osm1';
let pending = false;

// Fetch API
async function requestRoute(src, dst) {
    if (pending) throw new Error('Another route is computing…');
    pending = true;
    try {
        const res = await fetch('/api/route', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ name: GRAPH_NAME, src, dst })
        });
        if (!res.ok) {
            const payload = await res.json().catch(() => ({}));
            throw new Error(payload.error || `HTTP ${res.status}`);
        }
        return await res.json();
    } finally {
        pending = false;
    }
}

// Draw Route
function drawRoute(result) {
    routeLayer.clearLayers();
    shortcutLayer.clearLayers();

    // Normalize shapes:
    // djk → { path, cost? }
    // ch/cch → { actual_path, shortcut_path, cost? }
    const actual = result.actual_path || result.path || [];
    const shortcut = result.shortcut_path || [];

    const toCoords = (ids) =>
        ids.map(id => nodeById.get(id)).filter(Boolean).map(n => [n.lat, n.lon]);

    const actualCoords = toCoords(actual);
    const shortcutCoords = toCoords(shortcut);

    let bounds = null;

    if (actualCoords.length >= 2) {
        const poly = L.polyline(actualCoords, { weight: 5, color: COLORS.route, opacity: 0.95 }).addTo(routeLayer);
        bounds = poly.getBounds();
        // Start / End markers
        L.circleMarker(actualCoords[0], { radius: 6, weight: 2, color: COLORS.start, fillColor: COLORS.start, fillOpacity: 0.9 }).addTo(routeLayer)
            .bindTooltip('Start', { direction: 'top' }).addTo(routeLayer).bindTooltip('Start', { direction: 'top' });
        L.circleMarker(actualCoords[actualCoords.length - 1], { radius: 6, weight: 2, color: COLORS.end, fillColor: COLORS.end, fillOpacity: 0.9 }).addTo(routeLayer)
            .bindTooltip('End', { direction: 'top' }).addTo(routeLayer).bindTooltip('End', { direction: 'top' });
    }

    if (shortcutCoords.length >= 2) {
        const spoly = L.polyline(shortcutCoords, { weight: 3, opacity: 0.9, color: COLORS.shortcut, dashArray: '10 10' }).addTo(shortcutLayer);
        bounds = bounds ? bounds.extend(spoly.getBounds()) : spoly.getBounds();
    }

    if (bounds) map.fitBounds(bounds.pad(0.2));

    // Status line
    const costStr = typeof result.cost === 'number' ? ` • cost: ${result.cost}` : '';
    const algoStr = getAlgo().toUpperCase();
    const nActual = actual.length || 0;
    const nShortcut = shortcut.length || 0;
    const extra = nShortcut ? ` • shortcut nodes: ${nShortcut}` : '';
    statusEl.textContent = `${algoStr} • nodes: ${nActual}${extra}${costStr}`;
}

// Haversine great-circle distance between two [lat, lon] points, returns km
function haversineKm(a, b) {
    const toRad = d => (d * Math.PI) / 180;
    const R = 6371; // km
    const [lat1, lon1] = a;
    const [lat2, lon2] = b;
    const dLat = toRad(lat2 - lat1);
    const dLon = toRad(lon2 - lon1);
    const s1 = Math.sin(dLat / 2);
    const s2 = Math.sin(dLon / 2);
    const aa = s1 * s1 + Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * s2 * s2;
    const c = 2 * Math.atan2(Math.sqrt(aa), Math.sqrt(1 - aa));
    return R * c;
}

function getAlgo() {
    const el = document.getElementById('algo-select');
    return el ? el.value : 'djk';
}

// Request
async function requestRoute(src, dst) {
    const payload = { name: GRAPH_NAME, src, dst, algo: getAlgo() };
    const res = await fetch('/api/route', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
    });
    if (!res.ok) {
        const data = await res.json().catch(() => ({}));
        throw new Error(data.error || `HTTP ${res.status}`);
    }
    return res.json();
}
