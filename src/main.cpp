#include <iostream>
#include <graph.hpp>
#include <ch-graph.hpp>
#include <file-handler.hpp>
#include <sssp-result.hpp>
#include<pre-processing/contraction-hierarchy.hpp>
#include <shortest-path/spira.hpp>
#include <shortest-path/new-variant.hpp>
#include <benchmark_tests.hpp>

using namespace std;

int run_algorithms() {
    cout << "running CH on OSM 1" << endl;

    string filepath = "./Input_Data/SparseRoadNetworks/osm1.txt";
    int src, dst;

    // // Parse command line arguments
    // if (argc >= 2) {
    //     src = stoi(argv[1]);
    // }
    // if (argc >= 3) {
    //     dst = stoi(argv[2]);
    // }

    cerr << "Running Dijkstra on " << filepath
                  << " src=" << src << " dst=" << dst << endl;

        FileHandler fh;
        Graph graph = fh.read_sparse_graph_file(filepath);

        CH ch(graph);
        int shortcuts_added = ch.preprocess();

        cout << "Preprocessing complete. " << shortcuts_added << " shortcuts added." << endl;

        // ch.build_upward_adj();
        // ch.build_downward_adj();

        // Now run spira quesries on the upward and downward adjacency lists
         CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());

         chGraph.print_upward_adj();

        //  Graph graph_for_spira(chGraph.number_of_nodes());
        //  Graph graph_for_spira(chGraph.number_of_nodes());

         cout << "Building graph for Spira..." << endl;
        // graph_for_spira.print_adj();

        Dijkstra dijkstra(chGraph);
        SsspResult dijkstra_result = dijkstra.compute_shortest_path(src, dst);

        cout << "Dijkstra complete. Shortest path cost: " << dijkstra_result.total_cost << endl;
        // in SPT mode, print distances to nodes that are reachable from src
        for (size_t i = 0; i < dijkstra_result.distance.size(); ++i) {
            if (dijkstra_result.distance[i] < INF_COST) {
                cout << "Distance from " << src << " to " << i << ": " << dijkstra_result.distance[i] << endl;
            }
        }
        cout << "Number of PQ pops: " << dijkstra_result.number_of_pops << endl;

        // chGraph.print_adj_out();
        chGraph.sort_all_neighbors();
        Spira spira(chGraph);
         // Optional: print the rank order of nodes
        SsspResult spira_result = spira.compute_shortest_path(src, dst);

        cout << "Spira complete. Shortest path cost: " << spira_result.total_cost << endl;

        // in SPT mode, print distances to nodes that are reachable from src
        for (size_t i = 0; i < spira_result.distance.size(); ++i) {
            if (spira_result.distance[i] < INF_COST) {
                cout << "Distance from " << src << " to " << i << ": " << spira_result.distance[i] << endl;
            }
        }

        cout << "Number of PQ pops: " << spira_result.number_of_pops << endl;

        NewVariant newVariant(chGraph);
        SsspResult new_variant_result = newVariant.compute_shortest_path(src, dst);

        cout << "Spira complete. Shortest path cost: " << new_variant_result.total_cost << endl;

        // in SPT mode, print distances to nodes that are reachable from src
        for (size_t i = 0; i < new_variant_result.distance.size(); ++i) {
            if (new_variant_result.distance[i] < INF_COST) {
                cout << "Distance from " << src << " to " << i << ": " << new_variant_result.distance[i] << endl;
            }
        }

        cout << "Number of PQ pops: " << new_variant_result.number_of_pops << endl;


        
    return 0;
}

int main() {
    BenchmarkTests::run_benchmark_on_sparse_graphs(false);
}