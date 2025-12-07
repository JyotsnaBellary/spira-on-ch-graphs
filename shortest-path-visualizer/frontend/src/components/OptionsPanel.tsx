import { useGraph } from "../context/GraphContext";

export default function OptionsPanel() {
  const {
    algorithm,
    setAlgorithm,
    weightMode,
    setWeightMode
  } = useGraph();

  const algorithms = ["Dijkstra", "Spira", "NewVariant"];

  const weightModes = [
    "Original",
    "Exponential",
    "Uniform"
  ];

  return (
    <div>
      <h3>Options</h3>

      {/* Algorithm Dropdown */}
      <label>Algorithm:</label>
      <select
        value={algorithm}
        onChange={(e) => setAlgorithm(e.target.value)}
      >
        {algorithms.map((algo) => (
          <option key={algo} value={algo}>{algo}</option>
        ))}
      </select>

      <br /><br />

      {/* Weight Mode Dropdown */}
      <label>Weights:</label>
      <select
        value={weightMode}
        onChange={(e) => setWeightMode(e.target.value)}
      >
        {weightModes.map((w) => (
          <option key={w} value={w}>{w}</option>
        ))}
      </select>
    </div>
  );
}
