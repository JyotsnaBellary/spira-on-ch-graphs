// src/services/api.ts
export type AlgoRequest = {
  fileName: string;      // e.g. "osm1", "wi29"
  algorithm: string;     // "Dijkstra" | "Spira" | "NewVariant"
  weightMode: string;    // "Original" | "Exponential" | "Uniform"
  src: number;           // source node id
  dst: number;           // target node id, or -1 if you want full SPT
  graphType: string;     // "sparse" | "dense" (you can send "sparse" for osm*, "dense" for tsp)
};

// Call backedn API to run shortest path algorithm
export async function runAlgorithm(params: AlgoRequest) {
  const res = await fetch("http://localhost:5000/api/visualize", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(params),
  });

  if (!res.ok) {
    const errText = await res.text();
    throw new Error(`Backend error: ${res.status} ${errText}`);
  }

  // expect JSON with path, cost, etc.
  return res.json(); 
}
