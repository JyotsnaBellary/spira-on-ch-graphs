import express from "express";
import cors from "cors";
import { spawn } from "child_process";

const app = express();

// CORS – this alone handles GET, POST, OPTIONS correctly
app.use(cors({
  origin: "http://localhost:5173",
  methods: ["GET", "POST", "OPTIONS"],
  allowedHeaders: ["Content-Type"]
}));

app.use(express.json());

// ---- API ENDPOINT ----
app.post("/api/visualize", (req, res) => {
  const { fileName, algorithm, src, dst, weightMode, graphType } = req.body;

  console.log("Visualization request:", req.body);

  const args = [
    "visualize",
    fileName,
    algorithm,
    String(src),
    String(dst),
    weightMode,
    graphType
  ];

  const child = spawn("/home/jyotsna_bellary/mp/master-project/build/sssp_cli", args, {
    // cwd: process.cwd()
    cwd: "/home/jyotsna_bellary/mp/master-project" 
  });

  let stdout = "";
  let stderr = "";

  child.stdout.on("data", d => stdout += d.toString());
  child.stderr.on("data", d => stderr += d.toString());

  child.on("close", code => {
    if (code !== 0) {
      console.error("C++ error:", stderr);
      return res.status(500).json({ error: stderr });
    }

    try {
      const parsed = JSON.parse(stdout);
      res.json(parsed);
    } catch {
      res.json({ raw: stdout });
    }

  });
});

// ---- START SERVER ----
app.listen(5000, () => {
  console.log("Backend running at http://localhost:5000");
});
