const express = require('express');
const path = require('path');
const { execFile } = require('child_process'); // <-- add this

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

const CPP_BIN = path.resolve(__dirname, '/home/gio-lab/Jyotsna/efficient-routes/build/Debug/erp_cli');

// 2) Where your graph files live on the server
//    Put your file at: web-map/data/webmap.txt   (name="webmap")
const GRAPH_DIR = path.resolve(__dirname, 'data');

// simple guard
function toInt(x) {
  const n = Number(x);
  return Number.isInteger(n) ? n : NaN;
}
// / TOP (after app/static setup)
console.log('[server] pid=', process.pid, 'dir=', __dirname);

// Log every request so you can see if /api/health hits this server
app.use((req,res,next) => { console.log('[req]', req.method, req.url); next(); });

// Health route
app.get('/api/health', (req,res) => res.json({ ok: true }));
app.post('/api/route', (req, res) => {
  const name = String(req.body?.name || '').trim(); 
  const src = Number(req.body?.src);
  const dst = Number(req.body?.dst);
  const algo = String(req.body?.algo || 'djk').trim();

  if (!name) return res.status(400).json({ error: 'name required' });
  if (!Number.isInteger(src) || !Number.isInteger(dst)) {
    return res.status(400).json({ error: 'src and dst must be integers' });
  }

  // Map UI algo → CLI subcommand
  const cmdByAlgo = {
    djk: 'appquerydjk',
    ch: 'appquerych',
    cch: 'appquerycch',
  };
const subcmd = cmdByAlgo[algo];
  if (!subcmd) return res.status(400).json({ error: `unknown algo '${algo}'` });


const PROJECT_ROOT = path.resolve(__dirname, '..');
  const args = [subcmd, name, String(src), String(dst)];
console.log('[server] exec:', CPP_BIN, args.join(' '), 'cwd=', PROJECT_ROOT);

  // Run your C++ CLI: erp_cli <src> <dst>
  // Adjust args to match your program’s expected interface.
  execFile(CPP_BIN, args, { cwd: PROJECT_ROOT, timeout: 20_000 }, (err, stdout, stderr) => {
    if (err) {
      console.error('C++ error:', err, stderr);
      return res.status(500).json({ error: 'route computation failed' });
    }

    const out = stdout.trim();

    try {
  const parsed = JSON.parse(out);
  // Accept any of the supported shapes:
  if (
    (parsed.path && Array.isArray(parsed.path)) ||
    (parsed.actual_path && Array.isArray(parsed.actual_path)) ||
    (parsed.shortcut_path && Array.isArray(parsed.shortcut_path))
  ) {
    return res.json(parsed);
  }
} catch (_) {
  /* not JSON, fall back */
}

    // ---- Parse stdout from your program ----
    // Expect something like: "0 2 5 7" or "PATH: 0 2 5 7"
    const lines = out.split(/\r?\n/).map(l => l.trim());
    let pathIds = [];
    let cost = undefined;

    for (const L of lines) {
      if (/^cost\s*:/i.test(L)) {
        const m = L.split(':')[1];
        if (m) {
          const val = parseFloat(m.trim());
          if (Number.isFinite(val)) cost = val;
        }
      } else if (/^path\s*:/i.test(L) || /^[\d,\s]+$/.test(L)) {
        const tokens = L.replace(/^path\s*:/i, '').trim().split(/[\s,]+/);
        pathIds = tokens.map(x => parseInt(x, 10)).filter(Number.isFinite);
      }
    }

    if (pathIds.length) {
      return res.json(cost != null ? { path: pathIds, cost } : { path: pathIds });
    }

    // Couldn’t parse
    return res.status(422).json({ error: 'unrecognized solver output', raw: out });
  });
});



app.listen(PORT, () => {
  console.log(`Web map running at http://localhost:${PORT}`);
});