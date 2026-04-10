import { useEffect, useMemo, useState } from "react";
import { MapContainer, TileLayer, CircleMarker, Polyline, useMap } from "react-leaflet";
import { useGraph } from "../context/GraphContext";
import { runAlgorithm } from "../services/api";
import "leaflet/dist/leaflet.css";

// Zoom where nodes show up
function AutoFitMap({ coordinates }: { coordinates: [number, number][] }) {
  const map = useMap();

  useEffect(() => {
    if (!coordinates.length) return;
    const bounds = coordinates.map(([lat, lon]) => [lat, lon]);
    map.fitBounds(bounds as any, { padding: [40, 40], maxZoom: 12 });
  }, [coordinates, map]);

  return null;
}

// Renders the Leaflet map, handles node clicks to fetch SPT/paths, and manages status hints.
export default function MapViewer({ viewMode }: { viewMode: "sparse" | "dense" }) {
  const {
    coordinates,
    edges,
    fileData,
    algorithm,
    weightMode,
    src, setSrc,
    dst, setDst,
    spt, setSpt,
    path, setPath,
    pertinentEdges, setPertinentEdges,
    showPertinent,
    showPaths,
    status, setStatus,
    runtimes, setRuntimes,
  } = useGraph();
  const [statusMessage, setStatusMessage] = useState<string>(status || "Click a node to see its SPT.");

  // Check if sparse or dense graph uploaded
  const fileNameBase = fileData?.name.replace(/\.[^/.]+$/, "") || "";
  const graphType = fileNameBase.startsWith("osm") ? "sparse" : "dense";

  // Precompute a normalized layout for the node coordinates to draw them in SVG space.
  const planeLayout = useMemo(() => {
    if (!coordinates.length) {
      return { points: [] as { x: number; y: number }[], vbW: 1000, vbH: 800, pad: 40 };
    }

    const xs = coordinates.map((c) => c[1]);
    const ys = coordinates.map((c) => c[0]);
    const minX = Math.min(...xs);
    const maxX = Math.max(...xs);
    const minY = Math.min(...ys);
    const maxY = Math.max(...ys);
    const spanX = Math.max(maxX - minX, 1e-6);
    const spanY = Math.max(maxY - minY, 1e-6);

    const vbW = 1000;
    const vbH = Math.max(650, (spanY / spanX) * 1000 + 80);
    const pad = 40;

    function project(coord: [number, number]) {
      const xNorm = (coord[1] - minX) / spanX;
      const yNorm = (coord[0] - minY) / spanY;
      return {
        x: pad + xNorm * (vbW - 2 * pad),
        y: pad + (1 - yNorm) * (vbH - 2 * pad), // flip y here
      };
    }

    return {
      points: coordinates.map(project),
      vbW,
      vbH,
      pad,
    };
  }, [coordinates]);

  useEffect(() => {
    if (!coordinates.length) {
      setStatusMessage(status || "Loading graph...");
    } else {
      setStatusMessage("Click a node to see its SPT.");
    }
  }, [coordinates, status]);

  // Handle click request SPT or shortest-path
  async function handleNodeClick(nodeId: number) {
    // If a path is already shown, start fresh with this node as new source
    if (src !== null && dst !== null) {
      setSrc(nodeId);
      setDst(null);
      setPath([]);
      setSpt([]);
      setPertinentEdges([]);
      setRuntimes({
        algorithm,
        selected: null,
        dijkstra: null,
        spira: null,
        newVariant: null,
      });
      setStatusMessage("This is a rooted SPT. Click another node to find the shortest path to it.");

      // Run requested Algorithm
      const result = await runAlgorithm({
        fileName: fileNameBase,
        algorithm,
        weightMode,
        src: nodeId,
        dst: -1,
        graphType,
      });

      // Set SPT edges 
      if (result.spt_edges) setSpt(result.spt_edges);

      // set Pertinent Edges 
      if (result.pertinent_edges) setPertinentEdges(result.pertinent_edges);
      setRuntimes({
        algorithm,
        selected: result.runtime_ms ?? null,
        dijkstra: result.dijkstra_runtime_ms ?? null,
        spira: result.spira_runtime_ms ?? null,
        newVariant: result.newvariant_runtime_ms ?? null,
      });

      return;
    }

    // First click: choose source (compute SPT)
    if (src === null) {
      setSrc(nodeId);
      setDst(null);
      setPath([]);
      setSpt([]);
      setPertinentEdges([]);
      setRuntimes({
        algorithm,
        selected: null,
        dijkstra: null,
        spira: null,
        newVariant: null,
      });
      setStatusMessage("This is a rooted SPT. Click another node to find the shortest path to it.");

      const result = await runAlgorithm({
        fileName: fileNameBase,
        algorithm,
        weightMode,
        src: nodeId,
        dst: -1,
        graphType,
      });

      if (result.spt_edges) setSpt(result.spt_edges);
      if (result.pertinent_edges) setPertinentEdges(result.pertinent_edges);
      setRuntimes({
        algorithm,
        selected: result.runtime_ms ?? null,
        dijkstra: result.dijkstra_runtime_ms ?? null,
        spira: result.spira_runtime_ms ?? null,
        newVariant: result.newvariant_runtime_ms ?? null,
      });

      return;
    }

    // Second click: choose destination (compute SP)
    if (src != null && dst === null) {
      const currentSrc = src;
      setDst(nodeId);
      // Clear old highlights before fetching new path
      setPath([]);
      setPertinentEdges([]);
      setSpt([]);
      setRuntimes({
        algorithm,
        selected: null,
        dijkstra: null,
        spira: null,
        newVariant: null,
      });

      const result = await runAlgorithm({
        fileName: fileNameBase,
        algorithm,
        weightMode,
        src: currentSrc,
        dst: nodeId,
        graphType,
      });

      if (result.shortest_path) setPath(result.shortest_path);
      if (result.pertinent_edges) setPertinentEdges(result.pertinent_edges);
      setRuntimes({
        algorithm,
        selected: result.runtime_ms ?? null,
        dijkstra: result.dijkstra_runtime_ms ?? null,
        spira: result.spira_runtime_ms ?? null,
        newVariant: result.newvariant_runtime_ms ?? null,
      });

      // Keep selection highlighted until the next click starts a new selection
      setSrc(currentSrc);
      setDst(nodeId);
      setStatusMessage("Shortest path shown. Click a node to see its SPT.");
      return;
    }

    // Fallback reset (should rarely hit)
    setSrc(null);
    setDst(null);
    setPath([]);
    setSpt([]);
    setPertinentEdges([]);
    setStatusMessage("Click a node to see its SPT.");
  }

  const mapView = (
    <MapContainer
      key={`${fileNameBase}-${viewMode}`}
      center={[0, 0]}
      zoom={2}
      className="leaflet-map"
    >
      <TileLayer url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png" />

      <AutoFitMap coordinates={coordinates} />

      {/* Base edges (always visible in blue) */}
      {edges.map((e: any, i: number) => (
        <Polyline
          key={`edge-${i}`}
          positions={[coordinates[e.src], coordinates[e.trg]]}
          pathOptions={{
            color: "blue",
            weight: 4,
            opacity: 0.7,
          }}
        />
      ))}

      {/* SPT edges */}
      {showPaths && spt.map((e: any, i: number) => (
        <Polyline
          key={`spt-${i}`}
          positions={[coordinates[e.src], coordinates[e.trg]]}
          pathOptions={{ color: "#ff4d4f", weight: 4, opacity: 0.85 }}
        />
      ))}

      {/* Draw pertinent edges */}
      {showPertinent && pertinentEdges.map((e: any, i: number) => (
        <Polyline
          key={`pert-${i}`}
          positions={[coordinates[e.src], coordinates[e.trg]]}
          pathOptions={{
            color: e.in ? "#b56bff" : "#24d1c4",
            weight: 5,
            opacity: 1,
          }}
        />
      ))}

      {/* Draw shortest path */}
      {showPaths && path.length > 1 && (
        <Polyline
          positions={path.map((id) => coordinates[id])}
          pathOptions={{ color: "#ff4d4f", weight: 6, opacity: 0.95 }}
        />
      )}

      {/* Nodes */}
      {coordinates.map(([lat, lon], i) => (
        <CircleMarker
          key={i}
          center={[lat, lon]}
          radius={8}
          weight={3}
          eventHandlers={{ click: () => handleNodeClick(i) }}
          pathOptions={{
            color:
              src === i || dst === i ? "#8df78b" :
              "#555555",
            fillColor:
              src === i || dst === i ? "#4ac26b" :
              "#f2f2f2",
            fillOpacity: 0.92,
            opacity: 0.95,
            weight: 3,
          }}
        />
      ))}
    </MapContainer>
  );

  const planeView = (
    <div style={{ width: "100%", height: "100vh", background: "#f7f9fb" }}>
      <svg
        viewBox={`0 0 ${planeLayout.vbW} ${planeLayout.vbH}`}
        width="100%"
        height="100%"
        style={{ display: "block" }}
      >
        {/* Light base edges under everything */}
        <g stroke="#d0d7de" strokeWidth={3} opacity={0.3}>
          {edges.map((e: any, i: number) => {
            const a = planeLayout.points[e.src];
            const b = planeLayout.points[e.trg];
            if (!a || !b) return null;
            return <line key={`base-${i}`} x1={a.x} y1={a.y} x2={b.x} y2={b.y} />;
          })}
        </g>

        {/* Pertinent edges */}
        {showPertinent && (
          <g strokeWidth={3} opacity={0.9}>
            {pertinentEdges.map((e: any, i: number) => {
              const a = planeLayout.points[e.src];
              const b = planeLayout.points[e.trg];
              if (!a || !b) return null;
              return (
                <line
                  key={`pert-${i}`}
                  x1={a.x}
                  y1={a.y}
                  x2={b.x}
                  y2={b.y}
                  stroke={e.in ? "#b56bff" : "#24d1c4"}
                />
              );
            })}
          </g>
        )}

        {/* SPT edges */}
        {showPaths && (
          <g stroke="#ff4d4f" strokeWidth={3} opacity={0.85}>
            {spt.map((e: any, i: number) => {
              const a = planeLayout.points[e.src];
              const b = planeLayout.points[e.trg];
              if (!a || !b) return null;
              return <line key={`spt-${i}`} x1={a.x} y1={a.y} x2={b.x} y2={b.y} />;
            })}
          </g>
        )}

        {/* Shortest path */}
        {showPaths && path.length > 1 && (
          <polyline
            fill="none"
            stroke="#ff4d4f"
            strokeWidth={4}
            opacity={0.95}
            points={path
              .map((id) => {
                const pt = planeLayout.points[id];
                return pt ? `${pt.x},${pt.y}` : "";
              })
              .filter(Boolean)
              .join(" ")}
          />
        )}

        {/* Nodes on top */}
        <g>
          {planeLayout.points.map((pt, i) => (
            <g key={`node-${i}`} onClick={() => handleNodeClick(i)} style={{ cursor: "pointer" }}>
              <circle
                cx={pt.x}
                cy={pt.y}
                r={8}
                fill={
                  src === i || dst === i
                    ? "#4ac26b"
                    : "#f2f2f2"
                }
                stroke={
                  src === i || dst === i
                    ? "#8df78b"
                    : "#555555"
                }
                strokeWidth={3}
                opacity={0.96}
              />
            </g>
          ))}
        </g>
      </svg>
    </div>
  );

  return (
    <div style={{ position: "relative" }}>
      <div
        style={{
          position: "absolute",
          top: 12,
          left: "50%",
          transform: "translateX(-50%)",
          zIndex: 1100,
          background: "rgba(0,0,0,0.75)",
          color: "white",
          padding: "10px 14px",
          borderRadius: 10,
          boxShadow: "0 6px 12px rgba(0,0,0,0.35)",
          pointerEvents: "none",
          fontSize: 14,
        }}
      >
        {statusMessage}
      </div>

      {viewMode === "dense" ? planeView : mapView}
    </div>
  );
}
