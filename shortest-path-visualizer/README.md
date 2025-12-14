## Shortest Path Visualizer (Frontend)

- From `/frontend` run:
  ```bash
  npm install
  npm run dev
  ```
- Upload a TSP file to view algorithm performance on a dense graph (rendered on a plane).
- Upload an OSM `.txt` file to view performance on a sparse graph (rendered on a map).
- Choose the algorithm:
  - Dijkstra
  - Spira
  - NewVariant (enables pertinent edges toggle)
- Choose the edge weight distribution:
  - Exponential
  - Geometric (for dense graphs only)
  - Uniform (edge weights 1 for sparse graphs)
  - Uniform Random Distribution
- When using the NewVariant algorithm you can toggle display for in-pertinent and out-pertinent edges.
- Click one node to see the shortest-path tree (rooted there). Click a second node to see the shortest path between the two.
- A legend is provided on the side to distinguish edge types and node markers.

## Backend
- From `/backend` run:
  ```bash
  npm install
  node ./server.js
  ```
