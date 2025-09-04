# Efficient Route Planning Exam
**Student ID:** *1472532*
Report: Efficient_Route_Planning_Report.pdf
---

##Overview 
This project implements and benchmarks three shortest-path algorithms:

-**Dijkstra's Algorithm (DJ)**
-**Contraction Hierarchies (CH)**
-**Dijkstra's Algorithm (CCH)**

Additionally, it includes a **web application** that allows interactive querying of shortest paths between a source and destination using any of the three algorithms.

---

## Benchmarks

### Build Instructions
To run all benchmark tests:

```bash
KAHIP_NODE_ORDERING=../kahip_build/KaHIP/build/node_ordering ./build.sh
```
The path to Kahip must be set correctly to run benchmarks involving CCH, 
Otherwise the following can be run as well. 
RoadNetworks are to be put in ./RoadNetworks/ at the same level as src folder. 

```bash
mkdir build
./build.sh
```

### Running CLI Benchmarks  
The CLI binary is located at:  

```bash
erp/build/Debug/erp_cli
```

The CLI can be used with different arguments to run specific benchmarks:

Available Arguments

```bash
ch_100
```
Runs and updates pre-processing time of all instances to a csv file. 
Runs 100 Contraction Hierarchy queries on all instances and stores results like priority queue pops, running time, cost etc.

```bash
cch_100
```
Runs and updates separator-decomposition and pre-processing time, Shortcuts, Average and Maximum number of triangles for an edge for all instances to a csv file. 
Runs 100 Customizable Contraction Hierarchy queries on all instances and stores results like priority queue pops, running time, cost etc. 

```bash
ch
```
Runs a single CH benchmark on OSM5 for querying.

```bash
cch
```
Runs a single CCH benchmark on OSM5 for querying.

```bash
dijkstra
```
Runs a Dijkstra benchmark on OSM5 for querying.

```bash
appquerydjk $map $src $dst
```

Executes a Dijkstra query inside the web application backend.

map: Graph file (e.g., OSM1)

src: Source node ID

dst: Destination node ID

Example: 
```bash
./erp_cli appquerydjk OSM1 10 250
```
Additionally these are also available,
```bash
appquerych $map $src $dst

appquerycch $map $src $dst

```

## Web Application
```bash
npm run dev
```

Note:
The graph inside the app is set to OSM1. You must upload OSM! on the website for queries to work. No other graphs are compatible with the web app. 

Features:

You can query for shortest path using:
Dijkstra, 
CH (Contraction Hierarchies), 
CCH (Customizable Contraction Hierarchies)

