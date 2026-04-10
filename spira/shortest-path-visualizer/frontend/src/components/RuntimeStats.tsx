import { useMemo } from "react";
import { useGraph } from "../context/GraphContext";

function Bar({
  label,
  value,
  max,
  color,
}: {
  label: string;
  value: number | null;
  max: number;
  color: string;
}) {
  const pct = value && max > 0 ? Math.min(100, Math.round((value / max) * 100)) : 0;
  return (
    <div style={{ marginBottom: 1 }}>
      <div style={{ display: "flex", justifyContent: "space-between", fontSize: 12, color: "#ddd" }}>
        <span>{label}</span>
        <span>{value !== null && value >= 0 ? `${value.toFixed(1)} ms` : "—"}</span>
      </div>
      <div
        style={{
          height: 8,
          borderRadius: 6,
          background: "#2a2a2a",
          overflow: "hidden",
          border: "1px solid #444",
        }}
      >
        <div
          style={{
            width: `${pct}%`,
            height: "100%",
            background: color,
            transition: "width 0.2s ease",
          }}
        />
      </div>
    </div>
  );
}

export default function RuntimeStats() {
  const { algorithm, runtimes } = useGraph();

  const bars = useMemo(() => {
    const alg = algorithm;
    const list: { label: string; value: number | null; color: string }[] = [];

    if (alg === "Dijkstra") {
      list.push({ label: "Dijkstra", value: runtimes.dijkstra ?? runtimes.selected, color: "#8df78b" });
    } else if (alg === "Spira") {
      list.push({ label: "Spira", value: runtimes.spira ?? runtimes.selected, color: "#24d1c4" });
      list.push({ label: "Dijkstra", value: runtimes.dijkstra, color: "#8df78b" });
    } else if (alg === "NewVariant") {
      list.push({ label: "NewVariant", value: runtimes.newVariant ?? runtimes.selected, color: "#ffaf45" });
      list.push({ label: "Dijkstra", value: runtimes.dijkstra, color: "#8df78b" });
    }

    const maxVal = Math.max(...list.map((b) => (b.value ?? 0)));
    return { list, max: maxVal || 1 };
  }, [algorithm, runtimes]);

  if (!bars.list.length) return null;

  return (
    
    <div
      style={{
        background: "#1b1b1b",
        border: "1px solid #333",
        borderRadius: 10,
        padding: 10,
        marginBottom: 4,
        marginTop: 6
      }}
    >
      <div style={{ color: "white", fontSize: 13, marginBottom: 6 }}>
        Algorithm runtimes
      </div>
      {bars.list.map((b) => (
        <Bar key={b.label} label={b.label} value={b.value ?? null} max={bars.max} color={b.color} />
      ))}
    </div>
  );
}
