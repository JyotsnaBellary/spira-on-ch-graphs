# Master Project

In this project we compare and analyze three shortest-path algorithms on varied edge-weight distributions and graph topologies:
- Dijkstra
- Spira
- New Variant of SSSP algorithms (optimized PQ relaxation approach)

## Build
Configure the build directory once:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```
Run the CLI binary:
```bash
build/cli
```
After the initial CMake configure, you can optionally rebuild with:
```bash
./build.sh build
```

## Run and interact
For the best experience, use the Web app to interact with the algorithms.
- Backend:
  ```bash
  cd shortest-path-visualizer/backend
  node server.js
  ```
- Frontend:
  ```bash
  cd shortest-path-visualizer/frontend
  npm install
  npm run dev
  ```

To test the algorithms through the command line:
- CLI:
  ```bash
  build/cli interactive
  ```

### Web app features
- Choose benchmark mode by clicking one node (SPT) or two nodes (src–dst query).
- Select edge-weight distributions and pick the algorithm to run (Dijkstra, Spira, New Variant).
- Toggle between sparse and dense graphs (dense shown on a Euclidean plane for clarity).
- Optionally overlay pertinent edges and hide the path edges if you only want to see pertinent edges for a query.
- Hide pertinence edges if you only want the shortest path view for a query. 
- View runtime comparisons between Dijkstra and the other two algorithms for each query.
- A legend is given on the side that explains teh color coding of the edges. 

Note: While the application supports even large sparse and dense graphs, the query is slow as its heavy on the frontend. Hence best use with smaller dense and sparse graph examples
(Will make it faster in future updates :P)
