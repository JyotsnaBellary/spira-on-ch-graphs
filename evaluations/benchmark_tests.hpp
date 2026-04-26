// Test 1: as given in the paper using complete directed graphs. (they say with high probability, what does that even mean?)
// report results for SPT and for shortest path between src / trg pairs
// Test 2: use sparse graphs.
// report results for SPT and for shortest path between src / trg pairs
// Test 3: use TSP instances for dense graphs
// Measure runtime, number of edge relaxations 

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <set>
#include <cmath>
#include <random>
#include <unordered_set>
#include <vector>
#include <utility>
#include <cstdint>
#include <sssp-result.hpp>
#include <file-handler.hpp>
#include <shortest-path/dijkstra.hpp>
#include <shortest-path/spira.hpp>
#include <shortest-path/new-variant.hpp>
#include <ch-graph.hpp>
#include <pre-processing/contraction-hierarchy.hpp>
#include <types.hpp>

using namespace std;
using namespace filesystem;

// Struct used while building dense graphs with same number of edges as OSM for comparability
struct GraphCounts
{
    // --- nodes ---
    long long n = 0;

    // --- edges ---
    long long m = 0;
};

// Struct to maintain information on pertinent edges. 
struct PertinenceStats
{
    // --- Pertinence counts ---
    int total_pertinent_edges = 0;
    int total_out_pertinent_edges = 0;
    int total_in_pertinent_edges = 0;
    int total_in_pertinent_extracted_in_forward = 0;
    int conflict_both = 0;

    // Pertinent edges in SPT and shortest Path
    int total_spt_edges = 0;
    int out_spt = 0;
    int in_spt = 0;

    // --- Track conflicts ---
    int non_pertinent_edge_in_spt = 0;
    bool spt_edges_incorrectly_classified = false;

    // --- Ratios ---
    double ratio_pert_m = 0.0;         // total_pertinent_edges / m (number of edges)
    double ratio_pert_n = 0.0;         // total_pertinent_edges / n (number of vertices)
    double ratio_in_transferred = 0.0; // total_in_pertinent_extracted_in_forward / total_in_pertinent_edges
    double ratio_spt_out = 0.0;        // out_spt / total_spt_edges
    double ratio_spt_in = 0.0;         // in_spt / total_spt_edges
    double ratio_pop_pert = 0.0;       // no_of_pops / total_pert_edges
};

// Struct to maintain average over 100 queries
struct AggregateStats
{
    string filepath;
    int n = 0;
    int m = 0;

    // --- Number of queries per experiment---
    int runs = 0;

    // ---- Times ----
    double avg_d_time = 0;
    double avg_s_time = 0;
    double avg_nv_time = 0;

    // ---- Pops ----
    double avg_d_pops = 0;
    double avg_s_pops = 0;
    double avg_nv_pops = 0;
    double avg_nvq_pops = 0;

    // ------ Pops per m ------
    double avg_d_pops_to_m = 0;
    double avg_s_pops_to_m = 0;
    double avg_nv_pops_to_m = 0;   

    // ---- Pertinence counts ----
    double avg_in = 0;
    double avg_in_trans = 0;
    double avg_out = 0;
    double avg_pert = 0;

    // ---- Ratios ----
    double avg_ratio_pert_m = 0;
    double avg_ratio_pert_n = 0;
    double avg_ratio_in_trans = 0;
    double avg_ratio_spt_out = 0;
    double avg_ratio_spt_in = 0;
    double avg_ratio_pop_pert = 0;

    // ---- Suspicious runs ----
    int runs_with_conflict = 0;
    int runs_with_misclassified_spt = 0;

    // ---- Wrong results for shortest paths ----
    int mismatches = 0;
};

class BenchmarkTests
{
public:
    // ---- Helper Functions ----

    // Take the average of 100 runs
    static void finalize_aggregate_stats(AggregateStats &agg);

    // Append results to average.csv
    static void append_average_summary(const AggregateStats &A, const string &output_csv_path);

    // To compare if cost of one path is the same as the other 
    static bool same_cost(Cost a, Cost b);

    // Count if any node-wise mismatches between two SPTs (consistency checks)
    static int count_distance_mismatches(const vector<Cost> &a, const vector<Cost> &b);

    // Compare SPT equality between different algorithms (consistency checks)
    static bool compare_spt_results(const SsspResult &a,
                                    const SsspResult &b,
                                    const string &nameA,
                                    const string &nameB);

    // Record pertinence related information and flag any suspecious result for SPT
    static PertinenceStats analyze_spt_pertinence(const SsspResult &res, const CH_Graph &graph);
    static PertinenceStats analyze_spt_pertinence(const SsspResult &res, const Query_Graph_Type &reachable_query_graph, const CH_Graph &graph);
    
    static PertinenceStats analyze_path_pertinence(const SsspResult &res, const Query_Graph_Type &reachable_query_graph, const CH_Graph &graph);
    
    // Record pertinence related information and flag any suspecious result for shortest path queries
    static PertinenceStats analyze_path_pertinence(const SsspResult &res, const CH_Graph &graph);

    // ---- Print run stats ----
    static void print_run_stats(NodeId src, const CH_Graph &graph, const Query_Graph_Type &reachable_query_graph,
                     long long dijkstra_time_us, long long spira_time_us, long long new_variant_time_us,
                     const SsspResult &result_dijkstra, const SsspResult &result_spira, const SsspResult &result_new_variant,
                     const PertinenceStats &stats, int mismatch_count_ds, int mismatch_count_dn, bool mismatches);

    // Helper: Generate a 100 unique pairs of (src, dst) for benchmarking the algorithms
    static vector<pair<int, int>> generate_query_pairs(int n);

    // Reads  node count and  edge count from OSM.
    static GraphCounts read_osm_header_counts(const string &osm_path);

    // Return the node count needed for a complete graph based on target edges.
    static long long complete_graph_nodes_for_edges(long long target_edges);

    // // ---- Helper: Count node-wise mismatches between two SPTs ----
    // static int count_distance_mismatches(const vector<Cost> &a, const vector<Cost> &b);

    // // ---- Helper: Compare SPT equality ----
    // static bool compare_spt_results(const SsspResult &a,
    //                                 const SsspResult &b,
    //                                 const string &nameA,
    //                                 const string &nameB);

    // ---- Benchmark algorithms on src-dst queries ----
    static void run_src_dst_benchmark_on_CH_graph(CH_Graph &graph, const string &output_csv_path);

    // ---- Benchmark algorithms on SPT queries ----
    static void run_spt_benchmark_on_CH_graph(CH_Graph &graph, const string &output_csv_path);

    // ---- Wrappers to read and run ----

    // For Sparse Graphs
    static void process_sparse_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir);
    static int run_benchmark_on_sparse_graphs();

    // For Dense Graphs
    static void process_dense_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir);
    static int run_benchmark_on_dense_graphs();

    // Run Benchmark on synthetic graphs 9 million directed edges.
    static int run_benchmark_on_exponential_size_sweep(int min_n = 100,
                                                       int max_n = 3000,
                                                       int num_sizes = 10,
                                                       double lambda = 1.0,
                                                       uint64_t base_seed = 4242,
                                                       string base_output_dir = "output/exp_complete",
                                                       bool symmetric_bidirectional = false);

    // Run Benchmark on Synthetic graphs with same edges as the OSM graph datasets
    static void run_benchmark_dense_graph_osm_edges();
};
