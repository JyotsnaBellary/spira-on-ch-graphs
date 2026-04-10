#pragma once

#include <string>
#include <vector>
#include "file_handler.hpp"
#include "graph.hpp"
#include "types.hpp"
#include "dijkstra.hpp"
#include "spira.hpp"
#include "new_variant.hpp"
#include <chrono>

using Clock = chrono::high_resolution_clock;

// Free functions implemented in visualizer.cpp

// graph resolution and loading
string resolve_graph_file(const string& filename);
Graph load_graph(const string& path, const WeightMode& weightMode);

// run algorithms (single-source or SPT)
SsspResult run_dijkstra(Graph& graph, int src, int trg);
SsspResult run_spira(Graph& graph, int src, int trg);
SsspResult run_newvariant(Graph& graph, int src, int trg);

// debugging and correctness comparison
SsspResult compare_with_dijkstra(const SsspResult& algo_res,
                                 Graph& graph,
                                 int src,
                                 int trg);

// emit json for the web frontend
void emit_json(const SsspResult& result,
               const Graph& graph,
               int src,
               int trg,
               const string& algorithm,
               double dijkstra_runtime = -1,
               double spira_runtime = -1,
               double newvariant_runtime = -1);

// WebAPP namespace (visualization entry points)
namespace WebAPP {

    void visualize_algorithm(const string& filename,
                             const string& algorithm = "dijkstra",
                             int src = 0,
                             int trg = -1,
                             const WeightMode& weightMode = WeightMode::Original,
                             const string& graphType = "default");

    void visualize_dijkstra(const string& filename,
                            int src,
                            int trg,
                            const WeightMode& weightMode,
                            const string& algorithm);

    void visualize_spira(const string& filename,
                         int src,
                         int trg,
                         const WeightMode& weightMode,
                         const string& algorithm);

    void visualize_newvariant(const string& filename,
                              int src,
                              int trg,
                              const WeightMode& weightMode,
                              const string& algorithm);
}
