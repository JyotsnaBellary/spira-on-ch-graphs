import type { Control } from "leaflet";
import { useEffect, useState } from "react";
import FileUploader from "./components/FileUploader";
import MapViewer from "./components/MapViewer";
// import OptionsPanel from "./components/OptionsPanel";
import ControlsPanel from "./components/ControlPanels";
import Legend from "./components/Legend";
import RuntimeStats from "./components/RuntimeStats";
import { useGraph } from "./context/GraphContext";

export default function App() {
  // Track whether we're in the sparse (map) or dense (plane) view
  const [viewMode, setViewMode] = useState<"sparse" | "dense">("sparse");
  const { resetGraphOnFileChange, fileData } = useGraph();

  // Switch view; optionally reset overlays when user clicks the buttons
  function switchMode(mode: "sparse" | "dense", opts?: { reset?: boolean }) {
    if (mode === viewMode) return;
    setViewMode(mode);
    if (opts?.reset !== false) resetGraphOnFileChange();
  }

  // Auto-switch view based on uploaded file type (tsp → dense, txt → sparse)
  useEffect(() => {
    if (!fileData) return;
    const isDense = fileData.name.toLowerCase().endsWith(".tsp");
    const target = isDense ? "dense" : "sparse";
    if (target !== viewMode) {
      // Do not reset here; the upload already reset state
      switchMode(target, { reset: false });
    }
  }, [fileData, viewMode]);

  return (
    <div style={{ width: "100%", height: "100%", position: "relative" }}>
      
      {/* Floating UI panel */}
      <div
        className="floating-panel"
      >
        <h3 style={{ marginTop: 0 }}>Shortest Path Visualizer</h3>
        <div style={{ display: "flex", gap: 8, marginBottom: 12 }}>
          <button
            onClick={() => switchMode("sparse")}
            style={{
              flex: 1,
              padding: "8px 10px",
              borderRadius: 8,
              border: viewMode === "sparse" ? "2px solid #8df78b" : "1px solid #ccc",
              background: viewMode === "sparse" ? "#1f1f1f" : "#2a2a2a",
              color: "white",
              cursor: "pointer",
            }}
          >
            View for Sparse Graphs
          </button>
          <button
            onClick={() => switchMode("dense")}
            style={{
              flex: 1,
              padding: "8px 10px",
              borderRadius: 8,
              border: viewMode === "dense" ? "2px solid #8df78b" : "1px solid #ccc",
              background: viewMode === "dense" ? "#1f1f1f" : "#2a2a2a",
              color: "white",
              cursor: "pointer",
            }}
          >
            View for Dense Graphs
          </button>
        </div>
        <FileUploader />
        <RuntimeStats />
        <ControlsPanel />
        {/* <OptionsPanel /> */}
      </div>

      {/* Legend floating on the right */}
      <div
        style={{
          position: "absolute",
          top: 16,
          right: 16,
          zIndex: 1000,
        }}
      >
        <Legend />
      </div>

      {/* Fullscreen Map */}
      <MapViewer viewMode={viewMode} />
    </div>
  );
}
