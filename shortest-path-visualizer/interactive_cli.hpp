#pragma once

#include <string>
#include <graph.hpp>
#include <sssp_result.hpp>
#include <benchmark_tests.hpp>
#include <visualizer.hpp>

using namespace std;

// Algorithm choice enum
enum class AlgorithmChoice
{
    Dijkstra = 1,
    Spira,
    NewVariant,
    All,
    Exit = 0
};

// Graph type enum
enum class GraphType
{
    Dense = 1,
    Sparse
};

// Interactive CLI class
class interactive
{
private:
    // List of dense graph instances
    const vector<string> dense_graphs = {
        "wi29", "dj38", "qa194", "uy734", "zi929",
        "lu980", "rw1621", "mu1979", "nu3496",
        "ca4663", "tz6117", "ym7663",
        "pm8079", "ei8246", "kz9976"};

    // List of sparse graph instances
    const vector<string> sparse_graphs = {
        "osm1", "osm2", "osm3", "osm4", "osm5",
        "osm6", "osm7", "osm8", "osm9", "osm10", "osm11"};

public:
    // Choose to go back to menu
    bool go_back_to_main_menu();

    // Choose weight mode
    WeightMode choose_weight_mode();

    // Choose if SPT or single path
    bool choose_is_spt();

    // Choose graph instance
    string choose_graph_instance(const vector<string> &graphs);

    // Choose graph type
    GraphType choose_graph_type();

    // Choose algorithm
    AlgorithmChoice choose_algorithm();

    // Run the interactive menu
    void run_interactive_cli();

    // Helper functions
    void print_algorithm_summary(const string &name,
                                 const SsspResult &result,
                                 const Graph &graph,
                                 NodeId src,
                                 bool isSPT);

    // Print pertinence summary
    void print_pertinence_summary(const PertinenceStats &stats);

    // Compare path costs
    void compare_path_costs(const SsspResult &reference_result,
                            const SsspResult &test_result);
};
