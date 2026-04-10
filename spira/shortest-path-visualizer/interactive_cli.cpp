#include "interactive_cli.hpp"
#include <iomanip>
#include <iostream>

using namespace std;

// Compute total weight of SPT
Cost compute_spt_total_weight(const SsspResult& res, const Graph& graph, NodeId src)
{
    Cost total = 0;

    for (NodeId v = 0; v < res.via_edge.size(); ++v) {
        if (v == src) continue;

        EdgeId e = res.via_edge[v];
        if (e != INVALID_EDGE) {
            total += graph.get_edge(e).cost;
        }
    }

    return total;
}

// Choose to go back to main menu
bool interactive::go_back_to_main_menu() {
    cout << "\nWhat would you like to do next?\n";
    cout << "1. Back to main menu\n";
    cout << "0. Exit\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;
    return (choice == 1);
}

// Choose if SPT or single path
bool interactive::choose_is_spt() {
    cout << "\nChoose query type:\n";
    cout << "1. SPT (single-source)\n";
    cout << "2. s–t query\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;
    return (choice == 1);
}

// Choose weight mode
WeightMode interactive::choose_weight_mode() {
    cout << "\nChoose edge-weight distribution:\n";
    cout << "1. Original (Uniform for sparse, Geometric for Dens \e Graphs)\n";
    cout << "2. Exponential distribution\n";
    cout << "3. Unform Random Distribution\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;

    switch (choice) {
        case 1: return WeightMode::Original;
        case 2: return WeightMode::Exponential;
        case 3: return WeightMode::UniformRandomDistribution;
        default:
            cout << "Invalid choice, defaulting to Original.\n";
            return WeightMode::Original;
    }
}

// Choose graph instance
string interactive::choose_graph_instance(const vector<string>& graphs) {
    cout << "\nAvailable graph instances:\n\n";

    // Print two columns of all graph instances
    for (size_t i = 0; i < graphs.size(); i += 2) {
        cout << setw(2) << i + 1 << ". " << setw(10) << graphs[i];
        if (i + 1 < graphs.size())
            cout << setw(8) << i + 2 << ". " << graphs[i + 1];
        cout << "\n";
    }

    cout << "\nChoice: ";
    int choice;
    cin >> choice;

    return graphs.at(choice - 1);
}

// Choose graph type: sParse or Dense
GraphType interactive::choose_graph_type() {
    cout << "\nChoose graph type:\n";
    cout << "1. Dense (TSP)\n";
    cout << "2. Sparse (OSM)\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;
    return static_cast<GraphType>(choice);
}

// Choose algorithm: Dijkstra, Spira, NewVariant, All
AlgorithmChoice interactive::choose_algorithm() {
    cout << "\nChoose algorithm:\n";
    cout << "1. Dijkstra\n";
    cout << "2. Spira\n";
    cout << "3. New Variant\n";
    cout << "4. All algorithms\n";
    cout << "0. Exit\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;
    return static_cast<AlgorithmChoice>(choice);
}

// Print algorithm summary
void interactive::print_algorithm_summary(
    const string& name,
    const SsspResult& result,
    const Graph& graph,
    NodeId src,
    bool isSPT
) {
    cout << "\n========================================\n";
    cout << " Algorithm        : " << name << "\n";
    cout << " Query Type       : " << (isSPT ? "SPT" : "s–t") << "\n";
    cout << "----------------------------------------\n";

    cout << fixed << setprecision(3);
    cout << " Runtime (ms)     : " << result.runtime_ms << "\n";

    if (name == "New forward backward Variant" || name == "Spira") {
        cout << " Edge pops    : " << result.number_of_pops << "\n";
        
    } else {
        cout << " Node pops    : " << result.number_of_pops << "\n";
    }

    if (result.number_of_Q_pops > 0)
        cout << " Q pops           : " << result.number_of_Q_pops << "\n";

    cout << "----------------------------------------\n";

    if (isSPT) {
        Cost tree_weight = compute_spt_total_weight(result, graph, src);
        cout << " SPT total weight : " << tree_weight << "\n";
    } else {
        cout << " Path cost        : " << result.total_cost << "\n";
        cout << " Path             : ";
        for (auto node : result.path) cout << node << " ";
        cout << "\n";
    }

    cout << "========================================\n";
    cout << endl;
}



// Pertinence summary
void interactive::print_pertinence_summary(const PertinenceStats& stats)
{
    cout << "\n========================================\n";
    cout << " Pertinence Analysis\n";
    cout << "========================================\n";

    cout << " Total pertinent edges      : "
         << stats.total_pertinent_edges << "\n";
    cout << "   Out-pertinent edges      : "
         << stats.total_out_pertinent_edges << "\n";
    cout << "   In-pertinent edges       : "
         << stats.total_in_pertinent_edges << "\n";

    cout << "----------------------------------------\n";

    if (stats.conflict_both == 0) {
        cout << " Conflicts                 : None\n";
    } else {
        cout << " Conflicts                 : "
             << stats.conflict_both
             << " (WARNING)\n";
    }

    if (stats.non_pertinent_edge_in_spt == 0) {
        cout << " Non-pertinent SPT edges   : None\n";
    } else {
        cout << " Non-pertinent SPT edges   : "
             << stats.non_pertinent_edge_in_spt
             << " (WARNING)\n";
    }

    cout << "----------------------------------------\n";

    cout << fixed << setprecision(4);
    cout << " Pertinent / m             : "
         << stats.ratio_pert_m << "\n";
    cout << " Pertinent / n             : "
         << stats.ratio_pert_n << "\n";

    cout << "========================================\n\n";
}

// Compare against reference path
void interactive::compare_path_costs(const SsspResult& ref, const SsspResult& test)
{
    if (ref.total_cost != test.total_cost)
        cout << "cost mismatch\n";
    else {
        cout << endl;
        cout << "Costs Match" << endl;
    }

    cout << "\n";
}

// INTERACTIVE LOOP
void interactive::run_interactive_cli()
{
    cout << "=== Shortest Path Visualizer Interactive CLI ===\n";
    while (true) {

    // Choose Algorithm
    AlgorithmChoice algorithm = choose_algorithm();
    if (algorithm == AlgorithmChoice::Exit) return;

    // Choose Graph Type
    GraphType graphType = choose_graph_type();

    // Get available graphs based on type
    const auto& availableGraphs =
        (graphType == GraphType::Dense) ? dense_graphs : sparse_graphs;

    // Choose Graph Instance
    string graphFile = choose_graph_instance(availableGraphs);

    // Choose Weight Mode
    WeightMode weightMode = choose_weight_mode();

    // Get source and target
    int source, target;
    cout << "source: ";
    cin >> source;

    cout << "target (-1 for SPT): ";
    cin >> target;

    bool isSPT = (target < 0);

    // Load graph ONCE
    Graph graph =
        load_graph(resolve_graph_file(graphFile), weightMode);

    // Pre-sort neighbors for algorithms that need it
    graph.sort_all_neighbors();

    cout << "\nLoaded graph '" << graphFile
         << "' with " << graph.number_of_nodes()
         << " nodes and " << graph.number_of_edges()
         << " edges.\n";

    cout << "Running algorithms...\n";
    // Always run Dijkstra as reference
    SsspResult dijkstraResult =
        run_dijkstra(graph, source, target);

    cout << "\n--- Dijkstra Results ---\n";
    print_algorithm_summary("Dijkstra", dijkstraResult, graph, source, isSPT);

    if (algorithm == AlgorithmChoice::Dijkstra)
        continue;

    // Run Spira if chosen
    if (algorithm == AlgorithmChoice::Spira ||
        algorithm == AlgorithmChoice::All) {

        // Run Spira
        SsspResult spiraResult =
            run_spira(graph, source, target);

        cout << "\n--- Spira Results ---\n";
        print_algorithm_summary("Spira", spiraResult, graph, source, isSPT);
        compare_path_costs(dijkstraResult, spiraResult);
    }

    // Run New Variant if chosen
    if (algorithm == AlgorithmChoice::NewVariant ||
        algorithm == AlgorithmChoice::All) {

        // Run New Variant
        SsspResult newVariantResult =
            run_newvariant(graph, source, target);

        cout << "\n--- New Forward-Backward Variant Results ---\n";
        print_algorithm_summary("New forward backward Variant", newVariantResult, graph, source, isSPT);

        // Pertinence Analysis
        PertinenceStats pertinenceStats =
            (isSPT
                ? BenchmarkTests::analyze_spt_pertinence(
                      newVariantResult, graph)
                : BenchmarkTests::analyze_path_pertinence(
                      newVariantResult, graph));

        // Print Pertinence Summary
        print_pertinence_summary(pertinenceStats);

        // Compare costs if they match
        compare_path_costs(dijkstraResult, newVariantResult);
    }

    // ---- POST-RUN CHOICE ----
        bool go_back_to_menu = go_back_to_main_menu();
        
        if (!go_back_to_menu)
            return;
}



}
