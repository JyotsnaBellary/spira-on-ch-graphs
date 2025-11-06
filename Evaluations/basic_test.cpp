// #pragma once
#include "file_handler.hpp"
#include "dijkstra.hpp"
#include "spira.hpp"
#include "new_variant.hpp"
#include <chrono>

using namespace std;

void run_all_algorithms_on_sparse() {
    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file("./RoadNetworks/osm1.txt", true);

    // graph.print_adj_simple();

    auto time_algorithm = [](auto&& algorithm, int runs = 1) {
        std::vector<long long> times;
        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();
            auto result = algorithm();
            auto end = chrono::high_resolution_clock::now();
            times.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        return std::make_pair(times, times[0]); // return all times and first result time
    };

    Dijkstra dijkstra(graph);
    int src = 90;
    int trg = 9;
    // Dijkstra
    cout << "=== Dijkstra ===" << endl;
    auto [dijkstra_times, dijkstra_time] = time_algorithm([&]()
                                                          { return dijkstra.compute_shortest_path(src, trg); });
    DijkstraResult res = dijkstra.compute_shortest_path(src, trg);
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
    DijkstraResult res1 = spira.compute_shortest_path(src, trg);
    cout << "Total cost: " << res1.total_cost << endl;
    cout << "Number of pops: " << res1.number_of_pops << endl;
    cout << "Time: " << spira_time << " microseconds" << endl;
    cout << endl;

    // NewVariant
    NewVariant new_variant(graph);

    cout << "=== NewVariant ===" << endl;
    auto [newvar_times, newvar_time] = time_algorithm([&]()
                                                      { return new_variant.compute_shortest_path(src, trg); });
    DijkstraResult res2 = new_variant.compute_shortest_path(src, trg);
    cout << "Total cost: " << res2.total_cost << endl;
    cout << "Number of pops: " << res2.number_of_pops << endl;
    cout << "Time: " << newvar_time << " microseconds" << endl;
    cout << endl;
    return;
}


void run_all_algorithms_on_dense() {
    FileHandler fh;
    Graph graph = fh.read_dense_graph_file("./Input_Data/DenseNetworks/wi29.tsp", false);

    // graph.print_adj_simple();

    auto time_algorithm = [](auto&& algorithm, int runs = 1) {
        std::vector<long long> times;
        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();
            auto result = algorithm();
            auto end = chrono::high_resolution_clock::now();
            times.push_back(chrono::duration_cast<chrono::microseconds>(end - start).count());
        }
        return std::make_pair(times, times[0]); // return all times and first result time
    };

    Dijkstra dijkstra(graph);
    int src = 25;
    int trg = 10;
    for (auto& e : graph.get_out_neighbors(src)) {
    std::cout << src << " -> " << e << " cost=" << e << "\n";
    }

    // Dijkstra
    cout << "=== Dijkstra ===" << endl;
    auto [dijkstra_times, dijkstra_time] = time_algorithm([&]()
                                                          { return dijkstra.compute_shortest_path(src, trg); });
    DijkstraResult res = dijkstra.compute_shortest_path(src, trg);
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
    DijkstraResult res1 = spira.compute_shortest_path(src, trg);
    cout << "Total cost: " << res1.total_cost << endl;
    cout << "Number of pops: " << res1.number_of_pops << endl;
    cout << "Time: " << spira_time << " microseconds" << endl;
    cout << endl;

    // NewVariant
    NewVariant new_variant(graph);

    cout << "=== NewVariant ===" << endl;
    auto [newvar_times, newvar_time] = time_algorithm([&]()
                                                      { return new_variant.compute_shortest_path(src, trg); });
    DijkstraResult res2 = new_variant.compute_shortest_path(src, trg);
    cout << "Total cost: " << res2.total_cost << endl;
    cout << "Number of pops: " << res2.number_of_pops << endl;
    cout << "Time: " << newvar_time << " microseconds" << endl;
    cout << endl;
    return;
}