import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App";
import { GraphProvider } from "./context/GraphContext";
import "./index.css";

ReactDOM.createRoot(document.getElementById("root")!).render(
  <GraphProvider>
    <App />
  </GraphProvider>
);
