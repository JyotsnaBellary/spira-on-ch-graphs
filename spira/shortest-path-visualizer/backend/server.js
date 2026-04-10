import path from "path";
import { fileURLToPath } from "url";
import express from "express";
import cors from "cors";
import { spawn } from "child_process";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();

// CORS configuration
app.use(cors({
  origin: "http://localhost:5173",
  methods: ["GET", "POST", "OPTIONS"],
  allowedHeaders: ["Content-Type"]
}));

app.use(express.json());

// API endpoint to visualize shortest path
app.post("/api/visualize", (req, res) => {
  const { fileName, algorithm, src, dst, weightMode, graphType } = req.body;

  // ---- RESOLVE BINARY PATH ----
  const binaryPath =
    process.env.SSSP_CLI_PATH ||
    path.resolve(__dirname, "../../build/sssp_cli");

  console.log("Using SSSP binary:", binaryPath);

  const args = [
    "visualize",
    fileName,
    algorithm,
    String(src),
    String(dst),
    weightMode,
    graphType
  ];

  const child = spawn(binaryPath, args, {
    cwd: path.resolve(__dirname, "../..")
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
      res.json(JSON.parse(stdout));
    } catch {
      res.json({ raw: stdout });
    }
  });
});

// Start the server
app.listen(5000, () => {
  console.log("Backend running at http://localhost:5000");
});
