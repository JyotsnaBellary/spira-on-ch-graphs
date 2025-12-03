// Test 1: as give in the paper using complete directed graphs. (they say with high probability, what does that even mean?)
// report results for SPT and for shortest path between src / trg pairs
// Test 2: use sparse graphs.
// report results for SPT and for shortest path between src / trg pairs
// Test 3: use TSP instances for dense graphs

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
#include <sssp_result.hpp>
#include <file_handler.hpp>
#include <dijkstra.hpp>
#include <spira.hpp>
#include <new_variant.hpp>

using namespace std;
using namespace filesystem;

struct GraphCounts
{
    long long n = 0;
    long long m = 0;
};

struct PertinenceStats
{
    // Pertinence counts
    int total_pertinent_edges = 0;
    int total_out_pertinent_edges = 0;
    int total_in_pertinent_edges = 0;
    int total_in_pertinent_extracted_in_forward = 0;
    int conflict_both = 0;

    // SPT and Path analysis
    int total_spt_edges = 0;
    int out_spt = 0;
    int in_spt = 0;
    int non_pertinent_edge_in_spt = 0;
    bool spt_edges_incorrectly_classified = false;

    // --- Ratios ---
    double ratio_pert_m = 0.0;         // total_pertinent_edges / m
    double ratio_pert_n = 0.0;         // total_pertinent_edges / n
    double ratio_in_transferred = 0.0; // total_in_pertinent_extracted_in_forward / total_in_pertinent_edges
    double ratio_spt_out = 0.0;        // out_spt / total_spt_edges
    double ratio_spt_in = 0.0;         // in_spt / total_spt_edges
    double ratio_pop_pert = 0.0;       // no_of_pops / total_pert_edges
};

struct AggregateStats
{
    string filepath;
    int n = 0;
    int m = 0;

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
    static void finalize_aggregate_stats(AggregateStats &agg);
    static void append_average_summary(const AggregateStats &A, const string &output_csv_path);

    static PertinenceStats analyze_spt_pertinence(const SsspResult &res, const Graph &graph);
    static PertinenceStats analyze_path_pertinence(const SsspResult &res, const Graph &graph);

    // ---- Helper: Count node-wise mismatches between two SPTs ----
    static vector<pair<int, int>> generate_query_pairs(int n);

    // Reads  node count and  edge count from OSM.
    static GraphCounts read_osm_header_counts(const std::string &osm_path);

    // Return the node count needed for a complete graph based on target edges.
    static long long complete_graph_nodes_for_edges(long long target_edges);

    // ---- Helper: Count node-wise mismatches between two SPTs ----
    static int count_distance_mismatches(const std::vector<Cost> &a, const std::vector<Cost> &b);

    // ---- Helper: Compare SPT equality ----
    static bool compare_spt_results(const SsspResult &a,
                                    const SsspResult &b,
                                    const std::string &nameA,
                                    const std::string &nameB);

    // ---- Benchmark algorithms on src-dst queries ----
    static void run_src_dst_benchmark_on_graph(Graph &graph, const string &output_csv_path);

    // ---- Benchmark algorithms on SPT queries ----
    static void run_spt_benchmark_on_graph(Graph &graph, const std::string &output_csv_path);

    // ---- Wrapper to read and run ----

    // For Sparse Graphs
    static void process_sparse_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir);
    static void process_dense_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir);

    static int run_benchmark_on_sparse_graphs();
    static int run_benchmark_on_dense_graphs();

    static int run_benchmark_on_exponential_size_sweep(int min_n = 100,
                                                       int max_n = 3000,
                                                       int num_sizes = 10,
                                                       double lambda = 1.0,
                                                       uint64_t base_seed = 4242,
                                                       string base_output_dir = "output/exp_complete",
                                                       bool symmetric_bidirectional = false);

    

    static void run_benchmark_dense_graph_osm_edges();
};
