import { useGraph } from "../context/GraphContext";
// import { runAlgorithm } from "../services/api";

export default function FileUploader() {
  const { setCoordinates, setFileData, setEdges, setStatus, resetGraphOnFileChange } = useGraph();

  async function handleUpload(e: React.ChangeEvent<HTMLInputElement>) {
  const file = e.target.files?.[0];
  if (!file) return;

  // Clear previous graph state immediately and show loading
  resetGraphOnFileChange();
  setStatus(file.name.endsWith(".tsp") ? "Loading dense graph..." : "Loading graph...");
  setFileData(file);
  setStatus(file.name.endsWith(".tsp") ? "Loading dense graph..." : "Loading graph...");

  const text = await file.text();

  let coords: [number, number][] = [];
  let edges: { src: number; trg: number }[] = [];

  if (file.name.endsWith(".tsp")) {
    coords = parseTsp(text);

    // Build a complete graph for TSP
    edges = [];
    for (let i = 0; i < coords.length; i++) {
      for (let j = i + 1; j < coords.length; j++) {
        edges.push({ src: i, trg: j });
        edges.push({ src: j, trg: i });
      }
    }
  } 
  else {
    const result = parseOsm(text);
    coords = result.coords;
    edges = result.edges;
  }

  setCoordinates(coords);
  setEdges(edges);     // sets edges
}


  return (
    <div>
      <h3>Upload Graph File</h3>
      <input type="file" onChange={handleUpload} />
    </div>
  );

  
  // PARSE TSP FILES 
  function parseTsp(text: string): [number, number][] {
  const coords: [number, number][] = [];
  const lines = text.split("\n");
  let inSection = false;

  // function scaleValue(val: number) {
  //   const digits = Math.floor(Math.log10(Math.abs(val))) + 1;

  //   if (digits === 5) return val / 1000;
  //   if (digits === 4) return val / 100;
  //   if (digits === 3) return val / 10;
  //   return val; 
  // }

  for (let line of lines) {
    line = line.trim();

    if (line === "NODE_COORD_SECTION") {
      inSection = true;
      continue;
    }

    if (line === "EOF") break;

    if (inSection) {
      const parts = line.split(/\s+/);
      if (parts.length === 3) {
        const rawX = parseFloat(parts[1]);
        const rawY = parseFloat(parts[2]);

        // if (!isNaN(rawX) && !isNaN(rawY)) {
        //   const x = scaleValue(rawX);
        //   const y = scaleValue(rawY);

          // Leaflet expects [lat, lon]
          coords.push([rawX, rawY]);
        // }
      }
    }
  }

  return coords;
}

// Parse OSM files
function parseOsm(text: string): { coords: [number, number][], edges: {src: number, trg: number}[] } {
  const lines = text.trim().split("\n");
  if (lines.length < 3) return { coords: [], edges: [] };

  const nodeCount = parseInt(lines[0].trim(), 10);
  const edgeCount = parseInt(lines[1].trim(), 10);

  const coords: [number, number][] = [];

  // Parse nodes
  for (let i = 2; i < 2 + nodeCount; i++) {
    const parts = lines[i].trim().split(/\s+/);
    const lat = parseFloat(parts[1]);
    const lon = parseFloat(parts[2]);
    coords.push([lat, lon]);
  }

  // Parse edges (after nodes)
  const edges: { src: number; trg: number }[] = [];
  const start = 2 + nodeCount;
  const end = start + edgeCount;

  for (let i = start; i < end; i++) {
    const parts = lines[i].trim().split(/\s+/);
    const u = parseInt(parts[0]);
    const v = parseInt(parts[1]);

    edges.push({ src: u, trg: v });
    edges.push({ src: v, trg: u });  // bidirectional
  }

  return { coords, edges };
}


}
