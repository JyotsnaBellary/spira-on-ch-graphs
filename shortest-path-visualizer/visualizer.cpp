#include "visualizer.hpp"
#include <iostream>
// #include <dijkstra.hpp>
// #include <spira.hpp>
// #include <new_variant.hpp>
#include <unordered_map>
#include <benchmark_tests.hpp>
// // #include <benchmark_tests.cpp>
// // #include "benchmark_tests.cpp"
// // #include "json.hpp"   // your local header-only JSON file

// // using json = nlohmann::json;


using namespace std;
// struct PertinenceStats
// {
//     // Pertinence counts
//     int total_pertinent_edges = 0;
//     int total_out_pertinent_edges = 0;
//     int total_in_pertinent_edges = 0;
//     int total_in_pertinent_extracted_in_forward = 0;
//     int conflict_both = 0;

//     // SPT and Path analysis
//     int total_spt_edges = 0;
//     int out_spt = 0;
//     int in_spt = 0;
//     int non_pertinent_edge_in_spt = 0;
//     bool spt_edges_incorrectly_classified = false;

//     // --- Ratios ---
//     double ratio_pert_m = 0.0;         // total_pertinent_edges / m
//     double ratio_pert_n = 0.0;         // total_pertinent_edges / n
//     double ratio_in_transferred = 0.0; // total_in_pertinent_extracted_in_forward / total_in_pertinent_edges
//     double ratio_spt_out = 0.0;        // out_spt / total_spt_edges
//     double ratio_spt_in = 0.0;         // in_spt / total_spt_edges
//     double ratio_pop_pert = 0.0;       // no_of_pops / total_pert_edges
// };

// PertinenceStats analyze_spt_pertinence_here(
//     const SsspResult &res,
//     const Graph &graph)
// {
//     PertinenceStats stats;

//     for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid)
//     {
//         if (res.in_pertinent_edges[eid] and res.out_pertinent_edges[eid])
//         {
            
//             cerr << "Warning: edge " << eid
//                  << " is marked both in-pertinent and out-pertinent.\n";
//             stats.conflict_both += 1;
//         }
//         else if (res.in_pertinent_edges[eid])
//         {
//             stats.total_in_pertinent_edges += 1;
//             if (res.in_pertinent_edges_extracted_in_forward_phase[eid])
//             {
//                 stats.total_in_pertinent_extracted_in_forward += 1;
//             }
//         }
//         else if (res.out_pertinent_edges[eid])
//         {
//             stats.total_out_pertinent_edges += 1;
//         }
//     }

//     stats.total_pertinent_edges = stats.total_in_pertinent_edges + stats.total_out_pertinent_edges;
//     // cout << "Total conflicting edges: " << stats.conflict_both << endl;
//     // cout << "Analyzing SPT edges for pertinence...\n";
//     for (NodeId u = 0; u < (NodeId)res.via_edge.size(); ++u)
//     {
//         EdgeId eid = res.via_edge[u];

//         // skip root or unreachable
//         if (eid == INVALID_EDGE || eid == -1)
//             continue;

//         stats.total_spt_edges++;

//         bool is_out = res.out_pertinent_edges[eid];
//         bool is_in = res.in_pertinent_edges[eid];

//         if (is_out)
//             stats.out_spt++;
//         if (is_in)
//             stats.in_spt++;

//         // ---------- WARNING 1: SPT edge is neither in nor out ----------
//         if (!is_out && !is_in)
//         {
//             // Send warnings to stderr so stdout remains pure JSON for the web client
//             cerr << "WARNING: SPT edge " << eid
//                  << " (" << graph.get_edge(eid).src << " -> "
//                  << graph.get_edge(eid).trg
//                  << ") is NOT pertinent.\n";

//             // cout << "Cost: " << graph.get_edge(eid).cost << endl;
//             // cout << "Cost from src to this edge.src: " << res.distance[graph.get_edge(eid).src] << endl;
//             // cout << "Cost from src to this edge.trg: " << res.distance[graph.get_edge(eid).trg] << endl;
//             stats.non_pertinent_edge_in_spt++;
//         }
//     }

//     stats.spt_edges_incorrectly_classified = (stats.non_pertinent_edge_in_spt > 0);
//     if (stats.spt_edges_incorrectly_classified) {
//         // cout << "SPT edges are incorrectly classified.\n";
//     }
//     // ----- Ratios -----
//     if (graph.number_of_edges() > 0)
//     {
//         stats.ratio_pert_m = (double)stats.total_pertinent_edges / (double)graph.number_of_edges();
//     }
//     if (graph.number_of_nodes() > 0)
//     {
//         stats.ratio_pert_n = (double)stats.total_pertinent_edges / (double)graph.number_of_nodes();
//     }
//     if (stats.total_in_pertinent_edges > 0)
//     {
//         stats.ratio_in_transferred =
//             (double)stats.total_in_pertinent_extracted_in_forward /
//             (double)stats.total_in_pertinent_edges;
//     }
//     if (stats.total_spt_edges > 0)
//     {
//         stats.ratio_spt_out = (double)stats.out_spt / (double)stats.total_spt_edges;
//         stats.ratio_spt_in = (double)stats.in_spt / (double)stats.total_spt_edges;
//     }

//     return stats;
// }



    string resolve_graph_file(const string& filename)
{
    static const unordered_map<string, string> fileMap = {
        {"osm1", "./Input_Data/SparseRoadNetworks/osm1.txt"},
        {"osm2", "./Input_Data/SparseRoadNetworks/osm2.txt"},
        {"osm3", "./Input_Data/SparseRoadNetworks/osm3.txt"},
        {"osm4", "./Input_Data/SparseRoadNetworks/osm4.txt"},
        {"osm5", "./Input_Data/SparseRoadNetworks/osm5.txt"},
        {"osm6", "./Input_Data/SparseRoadNetworks/osm6.txt"},
        {"osm7", "./Input_Data/SparseRoadNetworks/osm7.txt"},
        {"osm8", "./Input_Data/SparseRoadNetworks/osm8.txt"},
        {"osm9", "./Input_Data/SparseRoadNetworks/osm9.txt"},
        {"osm10","./Input_Data/SparseRoadNetworks/osm10.txt"},
        {"osm11","./Input_Data/SparseRoadNetworks/osm11.txt"},

        {"wi29",   "./Input_Data/DenseNetworks/wi29.tsp"},
        {"dj38",   "./Input_Data/DenseNetworks/dj38.tsp"},
        {"uy734",  "./Input_Data/DenseNetworks/uy734.tsp"},
        {"rw1621", "./Input_Data/DenseNetworks/rw1621.tsp"},
        {"qa194",  "./Input_Data/DenseNetworks/qa194.tsp"},
        {"mu1979", "./Input_Data/DenseNetworks/mu1979.tsp"},
        {"lu980",  "./Input_Data/DenseNetworks/lu980.tsp"},
        {"zi929",  "./Input_Data/DenseNetworks/zi929.tsp"},
        {"nu3496", "./Input_Data/DenseNetworks/nu3496.tsp"}
    };
    auto it = fileMap.find(filename);
    if (it == fileMap.end()) {
        cerr << "[Visualizer] Unknown input graph name: " << filename << "\n";
        return "";
    }
    return it->second;
}

Graph load_graph(const string& path, const WeightMode& weightMode)
{
    FileHandler fh;

    bool isTxt = path.size() > 4 && path.substr(path.size() - 4) == ".txt";
    bool isTsp = path.size() > 4 && path.substr(path.size() - 4) == ".tsp";

    if (isTxt)
        return fh.read_sparse_graph_file(path, weightMode);

    if (isTsp)
        return fh.read_dense_graph_file(path, weightMode);

    cerr << "[Visualizer] Unsupported file extension: " << path << endl;
    return Graph();
}

SsspResult run_dijkstra(Graph& graph, int src, int trg)
{
    Dijkstra dijkstra(graph);

    auto start = Clock::now();
    SsspResult result = dijkstra.compute_shortest_path(src, trg);
    auto end = Clock::now();

    result.runtime_ms = chrono::duration<double, milli>(end - start).count();
    return result;
}

SsspResult run_spira(Graph& graph, int src, int trg)
{
    Spira spira(graph);

    graph.sort_all_neighbors();
    auto start = Clock::now();
    SsspResult result = spira.compute_shortest_path(src, trg);
    auto end = Clock::now();

    // Additional correctness check
    // SsspResult dijkstra_result = compare_with_dijkstra(result, graph, src, trg);

    result.runtime_ms = chrono::duration<double, milli>(end - start).count();
    return result;
}

SsspResult run_newvariant(Graph& graph, int src, int trg)
{
    NewVariant new_variant(graph);

    graph.sort_all_neighbors();

    auto start = Clock::now();
    SsspResult result = new_variant.compute_shortest_path(src, trg);
    auto end = Clock::now();

    result.runtime_ms = chrono::duration<double, milli>(end - start).count();

    // Additional correctness check
    // SsspResult dijkstra_result = compare_with_dijkstra(result, graph, src, trg);

    return result;
}


void emit_json(const SsspResult& result,
               const Graph& graph,
               int src,
               int trg,
               const string& algorithm,
               double dijkstra_runtime = -1,
               double spira_runtime = -1,
               double newvariant_runtime = -1)
{
    cout << "{";

    // ---------------------------------------------------
    // Basic info
    // ---------------------------------------------------
    cout << "\"src\":" << src << ",";
    cout << "\"dst\":" << trg << ",";
    cout << "\"total_cost\":" << result.total_cost << ",";

    // Runtime of the *selected* algorithm
    cout << "\"runtime_ms\":" << result.runtime_ms << ",";

    // ---------------------------------------------------
    // OPTIONAL: runtimes of other algorithms
    // ---------------------------------------------------
    cout << "\"dijkstra_runtime_ms\":" << dijkstra_runtime << ",";
    cout << "\"spira_runtime_ms\":" << spira_runtime << ",";
    cout << "\"newvariant_runtime_ms\":" << newvariant_runtime << ",";

    // ---------------------------------------------------
    // Pops and stats
    // ---------------------------------------------------
    cout << "\"number_of_pops\":" << result.number_of_pops << ",";
    cout << "\"number_of_Q_pops\":" << result.number_of_Q_pops << ",";
    cout << "\"redundant_pops\":" << result.redundant_pops << ",";
    cout << "\"avg_pops_per_node\":" << result.avg_pops_per_node << ",";

    // ---------------------------------------------------
    // SPT edges
    // ---------------------------------------------------
    cout << "\"spt_edges\":[";
    bool first = true;

    for (NodeId u = 0; u < (NodeId)result.via_edge.size(); ++u)
    {
        EdgeId eid = result.via_edge[u];
        if (eid < 0 || eid == INVALID_EDGE) continue;

        const Edge& e = graph.get_edge(eid);

        if (!first) cout << ",";
        cout << "{"
             << "\"src\":" << e.src << ","
             << "\"trg\":" << e.trg << ","
             << "\"eid\":" << eid << ","
             << "\"cost\":" << e.cost
             << "}";
        first = false;
    }
    cout << "],";

    // ---------------------------------------------------
    // Pertinent edges (NewVariant only)
    // ---------------------------------------------------
    if (algorithm == "NewVariant")
    {
        cout << "\"pertinent_edges\":[";
        first = true;

        for (EdgeId eid = 0; eid < graph.number_of_edges(); ++eid)
        {
            bool is_in  = result.in_pertinent_edges[eid];
            bool is_out = result.out_pertinent_edges[eid];
            if (!is_in && !is_out) continue;

            const Edge& e = graph.get_edge(eid);

            if (!first) cout << ",";
            cout << "{"
                 << "\"src\":" << e.src << ","
                 << "\"trg\":" << e.trg << ","
                 << "\"eid\":" << eid << ","
                 << "\"in\":"  << (is_in ? "true" : "false") << ","
                 << "\"out\":" << (is_out ? "true" : "false")
                 << "}";
            first = false;
        }
        cout << "],";
    }
    else {
        cout << "\"pertinent_edges\":[],";
    }

    // ---------------------------------------------------
    // Shortest path
    // ---------------------------------------------------
    cout << "\"shortest_path\":[";
    for (size_t i = 0; i < result.path.size(); ++i)
    {
        if (i) cout << ",";
        cout << result.path[i];
    }
    cout << "]";

    cout << "}" << endl;
}

SsspResult compare_with_dijkstra(const SsspResult& algo_res,
                           Graph& graph,
                           int src,
                           int trg)
{
    // Run reference Dijkstra
    // Dijkstra dijkstra(graph);
    // SsspResult result = dijkstra.compute_shortest_path(src, trg);
    SsspResult result = run_dijkstra(graph, src, trg);

    // Compare shortest path cost
    if (algo_res.total_cost != result.total_cost) {
        cerr << "[PathCheck] WARNING: Cost mismatch! "
             << "Algorithm cost = " << algo_res.total_cost
             << ", Dijkstra = " << result.total_cost << "\n";
    }

    // Compare actual path
    if (algo_res.path != result.path) {
        cerr << "[PathCheck] WARNING: PATH mismatch detected!\n";

        cerr << "Algorithm path: ";
        for (auto x : algo_res.path) cerr << x << " ";
        cerr << "\nDijkstra path: ";
        for (auto x : result.path) cerr << x << " ";
        cerr << "\n";
    }

    return result;
}



namespace WebAPP {
    void visualize_dijkstra(const string& filename, int src, int trg,
                        const WeightMode& mode, const string& algorithm)
{
    string path = resolve_graph_file(filename);
    if (path.empty()) return;

    Graph graph = load_graph(path, mode);

    SsspResult result = run_dijkstra(graph, src, trg);
    emit_json(result, graph, src, trg, algorithm);
}

    void visualize_spira(const string& filename, int src, int trg,
                     const WeightMode& mode, const string& algorithm)
{
    string path = resolve_graph_file(filename);
    if (path.empty()) return;

    Graph graph = load_graph(path, mode);

    SsspResult result = run_spira(graph, src, trg);
    SsspResult dijkstra_result = compare_with_dijkstra(result, graph, src, trg);

    emit_json(result, graph, src, trg, algorithm, dijkstra_result.runtime_ms, result.runtime_ms -1);
}

void visualize_newvariant(const string& filename, int src, int trg,
                          const WeightMode& mode, const string& algorithm)
{
    string path = resolve_graph_file(filename);
    if (path.empty()) return;

    Graph graph = load_graph(path, mode);

    SsspResult result = run_newvariant(graph, src, trg);
    PertinenceStats pertinenceStats;

    if (trg < 0) 
    {
        // No target: full SPT analysis
        pertinenceStats = BenchmarkTests::analyze_spt_pertinence(result, graph);
    }
    else
    {
        // Target exists: analyze only path to target
        pertinenceStats = BenchmarkTests::analyze_path_pertinence(result, graph);
    }

    SsspResult dijkstra_result = compare_with_dijkstra(result, graph, src, trg);

    emit_json(result, graph, src, trg, algorithm, dijkstra_result.runtime_ms, -1, result.runtime_ms);
}

void visualize_algorithm(const string& filename,
                         const string& algorithm,
                         int src,
                         int trg,
                         const WeightMode& mode,
                         const string& graphType)
{
    if (algorithm == "Dijkstra")
    {
        visualize_dijkstra(filename, src, trg, mode, algorithm);
    }
    else if (algorithm == "Spira")
    {
        visualize_spira(filename, src, trg, mode, algorithm);
    }
    else if (algorithm == "NewVariant")
    {
        visualize_newvariant(filename, src, trg, mode, algorithm);
    }
    else
    {
        cerr << "[Visualizer] ERROR: Unknown algorithm '" << algorithm << "'\n";
    }
}
} 
