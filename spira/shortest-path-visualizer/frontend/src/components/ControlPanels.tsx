import { useGraph } from "../context/GraphContext";


export default function ControlsPanel() {
  const { 
    algorithm, setAlgorithm,
    weightMode, setWeightMode,
    showPertinent, setShowPertinent,
    showPaths, setShowPaths,
    setPertinentEdges,
    path, spt,
    resetSelections,
  } = useGraph();

  return (
    <div>
      <h3>Controls</h3>

      {/* Algorithm change clears prior overlays when changed */}
      <label>Algorithm: </label>
      <select
        value={algorithm}
        onChange={(e) => {
          const next = e.target.value;
          setAlgorithm(next);
          // Clear pertinent edges on algorithm change and disable toggle for non-NewVariant
          setPertinentEdges([]);
          if (next !== "NewVariant") setShowPertinent(false);
          resetSelections();
        }}
      >
        <option value="Dijkstra">Dijkstra</option>
        <option value="Spira">Spira</option>
        <option value="NewVariant">NewVariant</option>
      </select>

      <br /><br />

      {/* Weight mode must resets overlays */}
      <label>Weight Mode: </label>
      <select
        value={weightMode}
        onChange={(e) => {
          setWeightMode(e.target.value);
          resetSelections();
        }}
      >
        <option value="Exponential">Exponential</option>
        <option value="Original">Original</option>
        <option value="Uniform">Uniform Random</option>
      </select>

      <br /><br />

      {/* Pertinent edges only apply to NewVariant */}
      <label style={{ display: "flex", alignItems: "center", gap: 8 }}>
        <input
          type="checkbox"
          disabled={algorithm !== "NewVariant"}
          checked={showPertinent && algorithm === "NewVariant"}
          onChange={(e) => setShowPertinent(e.target.checked)}
        />
        <span>Show pertinent edges</span>
      </label>

      {/* Single toggle controls both SP and SPT visibility */}
      <label style={{ display: "flex", alignItems: "center", gap: 8 }}>
        <input
          type="checkbox"
          checked={showPaths}
          disabled={path.length < 2 && !spt.length}
          onChange={(e) => setShowPaths(e.target.checked)}
        />
        <span>Show shortest path edges</span>
      </label>

      {/* <div style={{ marginTop: 8, color: "#ccc", fontSize: 12 }}>
        Tip: click once on a node for the SPT; click a second node to get the shortest path.
      </div> */}
    </div>
  );
}
