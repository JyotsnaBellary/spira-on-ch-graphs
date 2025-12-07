// #pragma once
#include "file_handler.hpp"
#include "dijkstra.hpp"
#include "spira.hpp"
#include "new_variant.hpp"
#include <chrono>

using namespace std;

static bool compare_and_print_spt_results(const SsspResult& a, const SsspResult& b, const string& nameA, const string& nameB)
{
    if (a.distance.size() != b.distance.size()) {
        cerr << nameA << " and " << nameB << " differ in size.\n";
        return false;
    }

    bool allMatch = true;
    for (size_t i = 0; i < a.distance.size(); ++i) {
        if (a.distance[i] != b.distance[i]) {
            cerr << "Distance mismatch at node " << i << ": "
                      << nameA << "=" << a.distance[i] << ", "
                      << nameB << "=" << b.distance[i] << "\n";
            allMatch = false;
        }
    }

    if (allMatch)
        cout << nameA << " and " << nameB << " distances match perfectly.\n";

    return allMatch;
}

void run_all_algorithms_on_sparse() {
    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file("./Input_Data/SparseRoadNetworks/osm1.txt", WeightMode::Exponential);

    // graph.print_adj_simple();

    auto time_algorithm = [](auto&& algorithm, int runs = 1) {
        vector<long long> times;
        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();
            auto result = algorithm();
            auto end = chrono::high_resolution_clock::now();
            times.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        return make_pair(times, times[0]); // return all times and first result time
    };

    Dijkstra dijkstra(graph);
    int src = 244;
    int trg = -1;
    // Dijkstra
    cout << "=== Dijkstra ===" << endl;
    auto [dijkstra_times, dijkstra_time] = time_algorithm([&]()
                                                          { return dijkstra.compute_shortest_path(src, trg); });
    SsspResult res = dijkstra.compute_shortest_path(src, trg);
    cout << "Total cost: " << res.total_cost << endl;
    cout << "Number of pops: " << res.number_of_pops << endl;
    cout << "Time: " << dijkstra_time << " microseconds" << endl;
    cout << endl;

    for(auto i = 0; i < res.path.size(); i++){
        cout << res.path[i] << " ";
    }
    cout << endl;

    // Spira
    graph.sort_all_neighbors();
    Spira spira(graph);

    cout << "=== Spira ===" << endl;
    auto [spira_times, spira_time] = time_algorithm([&]()
                                                    { return spira.compute_shortest_path(src, trg); });
    SsspResult res1 = spira.compute_shortest_path(src, trg);
    cout << "Total cost: " << res1.total_cost << endl;
    cout << "Number of pops: " << res1.number_of_pops << endl;
    cout << "Time: " << spira_time << " microseconds" << endl;
    cout << endl;

    // NewVariant
    NewVariant new_variant(graph);

    cout << "=== NewVariant ===" << endl;
    auto [newvar_times, newvar_time] = time_algorithm([&]()
                                                      { return new_variant.compute_shortest_path(src, trg); });
    SsspResult res2 = new_variant.compute_shortest_path(src, trg);
    cout << "Total cost: " << res2.total_cost << endl;
    cout << "Path length (#nodes): " << res2.path.size() << endl;
    cout << "Path edges (#edges): " << res2.edge_ids.size() << endl;
    cout << "Number of pops: " << res2.number_of_pops << endl;
    cout << "Time: " << newvar_times[0] << " us\n" << endl;

    //print the edge ids in the path
    cout << "Edge IDs in the path: ";
    for (auto eid : res2.edge_ids) {
        cout << eid << " ";
        Edge edge = graph.get_edge(eid);
        cout << "( " << edge.src << " -> " << edge.trg << " ),; ";
        cout << " cost: " << edge.cost;
        cout << " || ";
        if (res2.out_pertinent_edges[eid]) {
            cout << "[out-pertinent]";
        }
        if (res2.in_pertinent_edges[eid]) {
            cout << "[in-pertinent]";
        }
        cout << endl;
    }
    cout << endl << endl;
    // Count pertinent edges
    int in_pert  = 0;
    int out_pert = 0;
    int transfered = 0;

    for (EdgeId eid = 0; eid < graph.number_of_edges(); ++eid) {
        if (res2.in_pertinent_edges[eid]){
            in_pert++;
            // cout << "In-pertinent edge ID: " << eid << endl;
        }  
        if (res2.out_pertinent_edges[eid]) {
            out_pert++;
            // cout << "Out-pertinent edge ID: " << eid << endl;
        } 
        if (res2.in_pertinent_edges_extracted_in_forward_phase[eid]) {
            transfered++;
            // cout << "Transfered in-pertinent edge ID: " << eid << endl;
        } 

        if (res2.in_pertinent_edges[eid] && res2.out_pertinent_edges[eid]) {
            cout << "Warning: Edge ID " << eid << " is marked both in-pertinent and out-pertinent!" << endl;
        }
        if (res2.in_pertinent_edges_extracted_in_forward_phase[eid] && res2.out_pertinent_edges[eid]) {
            cout << "Warning: Edge ID " << eid << " is marked as transfered-pertinent and out-pertinent!" << endl;
        }
    }

    cout << "In-pertinent edges:  " << in_pert  << endl;
    cout << "Out-pertinent edges: " << out_pert << endl;
    cout << "transfered in-pertinent edges: " << transfered << endl;
    cout << "Total pertinent:     " << (in_pert + out_pert) << endl;
    cout << endl;


    if (trg < 0) {
        compare_and_print_spt_results(res, res1, "Dijkstra", "Spira");
        compare_and_print_spt_results(res, res2, "Dijkstra", "NewVariant");
    } 
    
    return;
}

void analyze_basic_spt_pertinence(
    const SsspResult& res,
    const Graph& graph)
{
    cout << "\n=== Pertinence analysis of SPT edges ===\n";

    int spt_out = 0;
    int spt_in = 0;
    int spt_transferred = 0;
    int total_spt_edges = 0;

    for (NodeId u = 0; u < res.parent.size(); ++u)
    {
        EdgeId eid = res.via_edge[u];

        if (eid == INVALID_EDGE || eid == -1) continue;  // root or unreachable
        total_spt_edges++;
        // cout << "SPT edge: " << eid << " (" << endl;
        //      << graph.get_edge(eid).src << " -> "
        //      << graph.get_edge(eid).trg << ")";
        const Edge& e = graph.get_edge(eid);
        if (e.id == INVALID_EDGE) cout << "ERROR: Edge with INVALID_EDGE id in SPT!\n";
        cout << "SPT edge: " << eid << " (" << e.src << " -> " << e.trg << ")";

        bool is_out = res.out_pertinent_edges[eid];
        bool is_in  = res.in_pertinent_edges[eid];
        bool is_trans = res.in_pertinent_edges_extracted_in_forward_phase[eid];

        if (is_out) {
            // cout << "  [out-pertinent]";
            spt_out++;
        }
        if (is_in) {
            // cout << "  [in-pertinent]";
            spt_in++;
        }
        if (is_trans) {
            // cout << "  [transferred-in]";
            spt_transferred++;
        }

        if (is_out && is_in) {
            cout << "  *** ERROR: BOTH PERTINENT ***";
        }

        cout << "\n";
    }

    cout << "\n--- SPT Pertinence Summary ---\n";
    cout << "Total SPT edges:              " << total_spt_edges << "\n";
    cout << "Out-pertinent SPT edges:      " << spt_out << "\n";
    cout << "In-pertinent SPT edges:       " << spt_in << "\n";
    cout << "Transferred in-pertinent:     " << spt_transferred << "\n";
    cout << "Total pertinent in SPT:       " << (spt_out + spt_in) << "\n";
    cout << "------------------------------\n\n";
}


void run_all_algorithms_on_dense() {
    FileHandler fh;
    Graph graph = fh.read_dense_graph_file("./Input_Data/DenseNetworks/wi29.tsp", WeightMode::Exponential);

    // graph.print_adj_simple();

    auto time_algorithm = [](auto&& algorithm, int runs = 1) {
        vector<long long> times;
        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();
            auto result = algorithm();
            auto end = chrono::high_resolution_clock::now();
            times.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        return make_pair(times, times[0]); // return all times and first result time
    };

    Dijkstra dijkstra(graph);
    int src = 18;
    int trg = -1;
    
    // Dijkstra
    cout << "=== Dijkstra ===" << endl;
    auto [dijkstra_times, dijkstra_time] = time_algorithm([&]()
                                                          { return dijkstra.compute_shortest_path(src, trg); });
    SsspResult res = dijkstra.compute_shortest_path(src, trg);
    cout << "Total cost: " << res.total_cost << endl;
    cout << "Number of pops: " << res.number_of_pops << endl;
    cout << "Time: " << dijkstra_time << " microseconds" << endl;
    cout << endl;

    // Spira
    graph.sort_all_neighbors();
    Spira spira(graph);

    cout << "=== Spira ===" << endl;
    auto [spira_times, spira_time] = time_algorithm([&]()
                                                    { return spira.compute_shortest_path(src, trg); });
    SsspResult res1 = spira.compute_shortest_path(src, trg);
    cout << "Total cost: " << res1.total_cost << endl;
    cout << "Number of pops: " << res1.number_of_pops << endl;
    cout << "Time: " << spira_time << " microseconds" << endl;
    cout << endl;

    // NewVariant
    NewVariant new_variant(graph);

    cout << "=== NewVariant ===" << endl;
    auto [newvar_times, newvar_time] = time_algorithm([&]()
                                                      { return new_variant.compute_shortest_path(src, trg); });
    SsspResult res2 = new_variant.compute_shortest_path(src, trg);
    cout << "Total cost: " << res2.total_cost << endl;

    cout << "Path length (#nodes): " << res2.path.size() << endl;
    cout << "Path edges (#edges): " << res2.edge_ids.size() << endl;
    cout << "Number of pops: " << res2.number_of_pops << endl;
    cout << "Time: " << newvar_times[0] << " us\n" << endl;

    //print the edge ids in the path
    cout << "Edge IDs in the path: ";
    for (auto eid : res2.edge_ids) {
        cout << eid << " ";
        Edge edge = graph.get_edge(eid);
        cout << "( " << edge.src << " -> " << edge.trg << " ),; ";
        cout << " cost: " << edge.cost;
        cout << " || ";
        if (res2.out_pertinent_edges[eid]) {
            cout << "[out-pertinent]";
        }
        if (res2.in_pertinent_edges[eid]) {
            cout << "[in-pertinent]";
        }
        cout << endl;
    }
    cout << endl << endl;
    // Count pertinent edges
    int in_pert  = 0;
    int out_pert = 0;
    int transfered = 0;

    for (EdgeId eid = 0; eid < graph.number_of_edges(); ++eid) {
        if (res2.in_pertinent_edges[eid]){
            in_pert++;
            // cout << "In-pertinent edge ID: " << eid << endl;
        }  
        if (res2.out_pertinent_edges[eid]) {
            out_pert++;
            // cout << "Out-pertinent edge ID: " << eid << endl;
        } 
        if (res2.in_pertinent_edges_extracted_in_forward_phase[eid]) {
            transfered++;
            // cout << "Transfered in-pertinent edge ID: " << eid << endl;
        } 

        if (res2.in_pertinent_edges[eid] && res2.out_pertinent_edges[eid]) {
            cout << "Warning: Edge ID " << eid << " is marked both in-pertinent and out-pertinent!" << endl;
        }
        if (res2.in_pertinent_edges_extracted_in_forward_phase[eid] && res2.out_pertinent_edges[eid]) {
            cout << "Warning: Edge ID " << eid << " is marked as transfered-pertinent and out-pertinent!" << endl;
        }
    }

    cout << "In-pertinent edges:  " << in_pert  << endl;
    cout << "Out-pertinent edges: " << out_pert << endl;
    cout << "transfered in-pertinent edges: " << transfered << endl;
    cout << "Total pertinent:     " << (in_pert + out_pert) << endl;
    cout << endl;


    if (trg < 0) {
        compare_and_print_spt_results(res, res1, "Dijkstra", "Spira");
        compare_and_print_spt_results(res, res2, "Dijkstra", "NewVariant");
    } 

    analyze_basic_spt_pertinence(res2, graph);
    
    return;
}

void run_all_algorithms_on_exponential_weights()
{
    FileHandler fh;
    Graph graph = fh.generate_complete_exponential_graph(200, 1.0, true, 42);

    int src = 0;
    int trg = -1;

    auto time_algorithm = [](auto&& algorithm, int runs = 1) {
        vector<long long> times;
        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();
            auto result = algorithm();
            auto end = chrono::high_resolution_clock::now();
            times.push_back(
                chrono::duration_cast<chrono::microseconds>(end - start).count()
            );
        }
        return make_pair(times, times[0]);
    };

    Dijkstra dijkstra(graph);
    cout << "=== Dijkstra ===" << endl;
    auto [dtimes, dtime] = time_algorithm([&]() { return dijkstra.compute_shortest_path(src, trg); });
    SsspResult dres = dijkstra.compute_shortest_path(src, trg);
    cout << "Total cost: " << dres.total_cost << endl;
    cout << "Number of pops: " << dres.number_of_pops << endl;
    cout << "Time: " << dtime << " microseconds" << endl << endl;

    graph.sort_all_neighbors();
    Spira spira(graph);
    cout << "=== Spira ===" << endl;
    auto [stimes, stime] = time_algorithm([&]() { return spira.compute_shortest_path(src, trg); });
    SsspResult sres = spira.compute_shortest_path(src, trg);
    cout << "Total cost: " << sres.total_cost << endl;
    cout << "Number of pops: " << sres.number_of_pops << endl;
    cout << "Time: " << stime << " microseconds" << endl << endl;

    NewVariant new_variant(graph);
    cout << "=== NewVariant ===" << endl;
    auto [ntimes, ntime] = time_algorithm([&]() { return new_variant.compute_shortest_path(src, trg); });
    SsspResult nres = new_variant.compute_shortest_path(src, trg);
    cout << "Total cost: " << nres.total_cost << endl;
    cout << "Number of pops: " << nres.number_of_pops << endl;
    cout << "Time: " << ntime << " microseconds" << endl << endl;
    if (trg < 0) {
        compare_and_print_spt_results(dres, sres, "Dijkstra", "Spira");
        compare_and_print_spt_results(dres, nres, "Dijkstra", "NewVariant");
    }
}

