#include <iostream>
#include <data_structures/graph.hpp>
#include <io/file_handler.hpp>
#include <algorithms/dijkstra.hpp>
#include <algorithms/CH.hpp>
#include <chrono> // for timing
#include <data_structures/ch_graph.hpp>

using namespace std;
using namespace std::chrono;

// forward-declared in main_stub.cpp
void erp_hello(){
    int a = 500;
    int b = 600;
    int c = a + b;
};


int main(int argc, char** argv) {
    // erp_hello();
    // cout << "erp_cli: ok\n";

    // //read input files
    // FileHandler fileHandler;
    // Graph graph = fileHandler.read_file("./data/osm4.txt");

    // cout << graph.num_edges();

    // //print adj list
    // // graph.print_adj_list();


    // //run dijkstra
    // Dijkstra dijkstra(graph);

    // DijkstraResult result = dijkstra.compute_shortest_path(1, 340);

    // cout << "Path of nodes: " << endl;
    // for (auto &node : result.path) {
    //     cout << node << " ";
    // }
    // cout << endl;

    // cout << "Total cost: " << result.total_cost << endl;

    //has an edge so returned edge with cost 1. 
    //if not then cost -1 and path is empty. that's when you need shortcut
    // pair<bool, ShortcutOpType> result1 = dijkstra.shortcut_search(1, 340, 17, 100000);
    // pair<bool, ShortcutOpType> result1 = dijkstra.shortcut_search(220, 340, -1, 100000);
    // pair<bool, ShortcutOpType> result1 = dijkstra.shortcut_search(207, 237, 262, 1);
    // cout << "Shortcut found: " << result1.first << endl;
    // cout << "Shortcut operation type: " << static_cast<int>(result1.second) << endl;

    // CH ch(graph);
    // // int result2 = ch.calculate_shortcuts(262);
    // // cout << "Number of shortcuts calculated: " << result2 << endl;

    // ch.preprocess();
    // cout << graph.num_edges() - ch.get_num_shortcuts() << endl;
    // cout << "Path of nodes: " << endl;
    // for (auto &node : result.path) {
    //     cout << node << " ";
    // }
    // cout << endl;

    // cout << "Total cost: " << result.total_cost << endl;

    vector<string> filepaths = {
         "./RoadNetworks/test.txt",
        //  "./RoadNetworks/osm1.txt",
        //  "./RoadNetworks/osm2.txt",
        //  "./RoadNetworks/osm3.txt",
        //  "./RoadNetworks/osm4.txt",
        //  "./RoadNetworks/osm5.txt",
        //  "./RoadNetworks/osm6.txt",
        //  "./RoadNetworks/osm7.txt",
        //  "./RoadNetworks/osm8.txt",
        //  "./RoadNetworks/osm9.txt",
        //  "./RoadNetworks/osm10.txt",
        //  "./RoadNetworks/osm11.txt",
     };

     // print shortcuts and time taken in minutes and seconds and ms for each input file name

     for (const auto &filepath : filepaths)
     {
          
          FileHandler fh;
          // Read the input graph
          Graph graph = fh.read_file(filepath);
          cout << "Finished reading graph from " << filepath << endl;
          // Preprocess the graph using Contraction Hierarchy

          // graph.printGraph();
          auto start_ch = high_resolution_clock::now();

          // fh.exportToCSV(graph, "nodes.csv", "edges.csv", {}, nullopt);
          // cout << "Finished exporting graph to CSV files." << endl;
          
          CH ch(graph);
          ch.preprocess();
          auto end_ch = high_resolution_clock::now();

          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_ch - start_ch);

          auto h = std::chrono::duration_cast<std::chrono::hours>(duration);
          duration -= h;

          auto m = std::chrono::duration_cast<std::chrono::minutes>(duration);
          duration -= m;

          auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
          duration -= s;

          auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

          std::cout << "Contraction Hierarchy preprocessing time: "
                    << h.count() << " h "
                    << m.count() << " m "
                    << s.count() << " s "
                    << ms.count() << " ms" << std::endl;

                    // Print the number of shortcuts added
                    // int shortcuts = ch.getEdgeCount() - graph.getEdgeCount();
                    // cout << "Number of shortcuts added: " << shortcuts / 2 << endl;
                    // cout << endl;

          //export path to CSV
          // fh.exportToCSV(graph, "nodes.csv", "edges.csv", {}, nullopt);
          // fh.runPythonVisualizer("../output_files/visualizations/plot_network.py");

          //create an instance of CH_graph, nodes, edges and rank order to be given to it 
        CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
        cout << "Created CH_Graph. Ready for Querying. " << "" << endl;
        cout << "Upward_edge_list:" << endl;
        chGraph.print_upward_adj();
        // cout << "enter src and dst:";
        // int src, dst;
        // cin >> src >> dst;

        // Perform query on chGraph. Initialise CH Dijkstra
        // CH_Dijkstra chDijkstra(chGraph);
        // chDijkstra.run(src, dst);
    }

    return 0;
}
