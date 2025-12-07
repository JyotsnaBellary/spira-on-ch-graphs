type LegendItem = {
  label: string;
  color: string;
  kind: "line" | "dot";
};

// Legend items
const items: LegendItem[] = [
  { label: "Non-pertinent edge", color: "blue", kind: "line" },
  { label: "SPT edge", color: "#ff4d4f", kind: "line" },
  { label: "Out-pertinent edge", color: "#24d1c4", kind: "line" },
  { label: "In-pertinent edge", color: "#b56bff", kind: "line" },
  { label: "Shortest path", color: "#ff4d4f", kind: "line" },
  { label: "Source node", color: "#8df78b", kind: "dot" },
  { label: "Destination node", color: "#8df78b", kind: "dot" },
];

export default function Legend() {
  return (
    <div
      style={{
        marginTop: 12,
        background: "rgba(0, 0, 0, 0.78)",
        border: "1px solid rgba(255,255,255,0.1)",
        borderRadius: 10,
        padding: "12px 14px",
        boxShadow: "0 6px 16px rgba(0,0,0,0.35)",
        minWidth: 190,
        color: "white",
      }}
    >
      <h4 style={{ margin: "0 0 6px", color: "white" }}>Legend</h4>
      <div style={{ display: "grid", gap: 6 }}>
        {items.map((item) => (
          <div
            key={item.label}
            style={{ display: "flex", alignItems: "center", gap: 8 }}
          >
            {item.kind === "line" ? (
              <div
                style={{
                  width: 28,
                  height: 4,
                  backgroundColor: item.color,
                  borderRadius: 2,
                }}
              />
            ) : (
              <div
                style={{
                  width: 14,
                  height: 14,
                  backgroundColor: item.color,
                  borderRadius: "50%",
                  border: "1px solid #333",
                }}
              />
            )}
            <span style={{ fontSize: 13 }}>{item.label}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
