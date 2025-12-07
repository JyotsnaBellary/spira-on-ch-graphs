import { createContext, useContext, useState, useEffect } from "react";
import type { ReactNode } from "react";

//  TYPE DEFINITIONS
export type GraphContextType = {
  fileData: File | null;
  setFileData: (f: File | null) => void;

  coordinates: [number, number][];
  setCoordinates: (c: [number, number][]) => void;

  edges: { src: number; trg: number }[];          // <-- added
  setEdges: (e: { src: number; trg: number }[]) => void;  // <-- added

  algorithm: string;
  setAlgorithm: (a: string) => void;

  weightMode: string;
  setWeightMode: (w: string) => void;

  src: number | null;
  setSrc: (s: number | null) => void;

  dst: number | null;
  setDst: (d: number | null) => void;

  path: number[];
  setPath: (p: number[]) => void;

  spt: any[];
  setSpt: (s: any[]) => void;

  pertinentEdges: any[];
  setPertinentEdges: (p: any[]) => void;

  showPertinent: boolean;
  setShowPertinent: (v: boolean) => void;

  showPaths: boolean;
  setShowPaths: (v: boolean) => void;

  status: string;
  setStatus: (s: string) => void;

  resetSelections: () => void;
  resetGraphOnFileChange: () => void;

  runtimes: {
    algorithm: string | null;
    selected: number | null;
    dijkstra: number | null;
    spira: number | null;
    newVariant: number | null;
  };
  setRuntimes: (r: GraphContextType["runtimes"]) => void;
};

// CREATE CONTEXT
const GraphContext = createContext<GraphContextType | null>(null);

//  PROVIDER
export function GraphProvider({ children }: { children: ReactNode }) {
  const [fileData, setFileData] = useState<File | null>(null);
  const [coordinates, setCoordinates] = useState<[number, number][]>([]);

  const [edges, setEdges] = useState<{ src: number; trg: number }[]>([]); // <-- added

  const [algorithm, setAlgorithm] = useState("NewVariant");
  const [weightMode, setWeightMode] = useState("Exponential");

  const [src, setSrc] = useState<number | null>(null);
  const [dst, setDst] = useState<number | null>(null);

  const [path, setPath] = useState<number[]>([]);
  const [spt, setSpt] = useState<any[]>([]);
  const [pertinentEdges, setPertinentEdges] = useState<any[]>([]);
  const [showPertinent, setShowPertinent] = useState(false);
  const [showPaths, setShowPaths] = useState(true);
  const [status, setStatus] = useState<string>("Upload a file to load nodes.");
  const [runtimes, setRuntimes] = useState<GraphContextType["runtimes"]>({
    algorithm: null,
    selected: null,
    dijkstra: null,
    spira: null,
    newVariant: null,
  });

  // Reset only user selections and overlays (keeps the currently loaded graph data)
  function resetSelections() {
    setSrc(null);
    setDst(null);
    setPath([]);
    setSpt([]);
    setPertinentEdges([]);
    setShowPertinent(false);
    setShowPaths(true);
    setStatus("Click a node to see its SPT.");
    setRuntimes({
      algorithm: null,
      selected: null,
      dijkstra: null,
      spira: null,
      newVariant: null,
    });
  }

  // Reset everything when a new file is uploaded 
  function resetGraphOnFileChange() {
    setCoordinates([]);
    setEdges([]);           
    resetSelections();
    setFileData(null);
    setStatus("Upload a file to load nodes.");
  }

  // Clear pertinent edges if algorithm changes away from NewVariant
  useEffect(() => {
    if (algorithm !== "NewVariant") {
      setPertinentEdges([]);
      setShowPertinent(false);
    }
  }, [algorithm]);

  return (
    <GraphContext.Provider
      value={{
        fileData, setFileData,
        coordinates, setCoordinates,

        edges, setEdges,         

        algorithm, setAlgorithm,
        weightMode, setWeightMode,

        src, setSrc,
        dst, setDst,

        path, setPath,
        spt, setSpt,

        pertinentEdges, setPertinentEdges,

        showPertinent, setShowPertinent,

        showPaths, setShowPaths,

        status, setStatus,

        runtimes, setRuntimes,

        resetSelections,
        resetGraphOnFileChange,
      }}
    >
      {children}
    </GraphContext.Provider>
  );
}

// HOOK
export function useGraph() {
  const ctx = useContext(GraphContext);
  if (!ctx) throw new Error("useGraph must be used inside GraphProvider");
  return ctx;
}
