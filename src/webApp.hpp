#include <data_structures/graph.hpp>
#include <io/file_handler.hpp>
#include <algorithms/dijkstra.hpp>
#include <algorithms/CH.hpp>
#include <data_structures/ch_graph.hpp>
#include <algorithms/ch_dijkstra.hpp>
#include <fstream>
#include <algorithms/CCH.hpp>
#include <string>
#include <sys/stat.h> 
#include <filesystem>

using namespace std;
using namespace chrono;


// This class has been introduced to support the web application with Querying the 
// required paths with the three algorithms.
class WebAPP
{
public:
    static void run_Dijkstra_Query(const string &map, NodeId src, NodeId dst)
    {
        // Map short names to actual file paths
        static const unordered_map<string, string> path_map = {
            {"test", "./RoadNetworks/test.txt"},
            {"testcch", "./RoadNetworks/testcch.txt"},
            {"osm1", "./RoadNetworks/osm1.txt"},
            {"osm2", "./RoadNetworks/osm2.txt"},
            {"osm3", "./RoadNetworks/osm3.txt"},
            {"osm4", "./RoadNetworks/osm4.txt"},
            {"osm5", "./RoadNetworks/osm5.txt"},
            {"osm6", "./RoadNetworks/osm6.txt"},
            {"osm7", "./RoadNetworks/osm7.txt"},
            {"osm8", "./RoadNetworks/osm8.txt"},
            {"osm9", "./RoadNetworks/osm9.txt"},
            {"osm10", "./RoadNetworks/osm10.txt"},
            {"osm11", "./RoadNetworks/osm11.txt"},
        };

        auto it = path_map.find(map);
        if (it == path_map.end())
        {
            cerr << "Unknown map choice: " << map << endl; // stderr
            return;                                                  // non-zero = error
        }

        const string filepath = it->second;
        cerr << "Running Dijkstra on " << filepath
                  << " src=" << src << " dst=" << dst << endl;

        FileHandler fh;
        Graph graph = fh.read_file(filepath);

        Dijkstra dijkstra(graph);
        auto result = dijkstra.compute_shortest_path(src, dst);
        // Assume result.path is vector<NodeId>. Adjust to your actual type.
        const auto &path = result.path;

        // ---- STRICT JSON output to stdout ----
        cout << "{\"algo\":\"djk\",\"path\":[";
        for (size_t i = 0; i < path.size(); ++i)
        {
            if (i)
                cout << ",";
            cout << path[i];
        }
        cout << "],\"cost\":" << result.total_cost << "}" << endl;
        return;
    }
    // print or use result

    static int run_CH_Dijkstra_Query(const string &map, NodeId src, NodeId dst)
    {
        // Map short names to actual file paths
        static const unordered_map<string, string> path_map = {
            {"test", "./RoadNetworks/test.txt"},
            {"testcch", "./RoadNetworks/testcch.txt"},
            {"osm1", "./RoadNetworks/osm1.txt"},
            {"osm2", "./RoadNetworks/osm2.txt"},
            {"osm3", "./RoadNetworks/osm3.txt"},
            {"osm4", "./RoadNetworks/osm4.txt"},
            {"osm5", "./RoadNetworks/osm5.txt"},
            {"osm6", "./RoadNetworks/osm6.txt"},
            {"osm7", "./RoadNetworks/osm7.txt"},
            {"osm8", "./RoadNetworks/osm8.txt"},
            {"osm9", "./RoadNetworks/osm9.txt"},
            {"osm10", "./RoadNetworks/osm10.txt"},
            {"osm11", "./RoadNetworks/osm11.txt"},
        };

        auto it = path_map.find(map);
        if (it == path_map.end())
        {
            cerr << "Unknown map choice: " << map << endl; // stderr
            return 2;                                                // non-zero = error
        }

        const string filepath = it->second;
        cerr << "Running Dijkstra on " << filepath
                  << " src=" << src << " dst=" << dst << endl;

        FileHandler fh;
        Graph graph = fh.read_file(filepath);

        CH ch(graph);
        ch.preprocess();

        CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
        CH_Dijkstra chDijkstra(chGraph);

        CH_DijkstraResult result = chDijkstra.compute_shortest_path(src, dst);
        result = chGraph.unpack_shortcuts(result);
        // Assume result.path is vector<NodeId>. Adjust to your actual type.
        // Your result fields — adjust names if your struct differs:
        const vector<NodeId> &actual_path = result.path;      // full expanded path
        const vector<NodeId> &shortcut_path = result.ch_path; // upward/downward CH hops
        const double cost = result.total_cost;                     // total cost (distance/time)

        // Basic sanity: if there is clearly no path, return non-zero (so server 500s)
        if (actual_path.empty())
        {
            cerr << "CH: no path found.\n";
            return 3;
        }

        // --- JSON to stdout ONLY ---
        cout << "{\"algo\":\"ch\",\"actual_path\":[";
        for (size_t i = 0; i < actual_path.size(); ++i)
        {
            if (i)
                cout << ',';
            cout << actual_path[i];
        }
        cout << "],\"shortcut_path\":[";
        for (size_t i = 0; i < shortcut_path.size(); ++i)
        {
            if (i)
                cout << ',';
            cout << shortcut_path[i];
        }
        cout << "],\"cost\":" << cost << "}\n";

        return 0;
    }
    // print or use result

    static int run_CCH_Dijkstra_Query(const string &map, NodeId src, NodeId dst)
    {
        // Map short names to actual file paths
        static const unordered_map<string, string> path_map = {
            {"test", "./RoadNetworks/test.txt"},
            {"testcch", "./RoadNetworks/testcch.txt"},
            {"osm1", "./RoadNetworks/osm1.txt"},
            {"osm2", "./RoadNetworks/osm2.txt"},
            {"osm3", "./RoadNetworks/osm3.txt"},
            {"osm4", "./RoadNetworks/osm4.txt"},
            {"osm5", "./RoadNetworks/osm5.txt"},
            {"osm6", "./RoadNetworks/osm6.txt"},
            {"osm7", "./RoadNetworks/osm7.txt"},
            {"osm8", "./RoadNetworks/osm8.txt"},
            {"osm9", "./RoadNetworks/osm9.txt"},
            {"osm10", "./RoadNetworks/osm10.txt"},
            {"osm11", "./RoadNetworks/osm11.txt"},
        };

        auto it = path_map.find(map);
        if (it == path_map.end())
        {
            cerr << "Unknown map choice: " << map << endl; // stderr
            return 2;                                                // non-zero = error
        }

        const string filepath = it->second;
        cerr << "Running Dijkstra on " << filepath
                  << " src=" << src << " dst=" << dst << endl;

        FileHandler fh;
        Graph graph = fh.read_file(filepath);

        CCH cch(graph);
        cch.compute_contraction_order();
        CCH_Result cch_res = cch.preprocess();
        cch.customization();

        CH_Graph cchGraph(cch.get_graph().get_all_nodes(), cch.get_graph().get_all_edges(), cch.get_ranks());
        CH_Dijkstra chDijkstra(cchGraph);
        CH_DijkstraResult result = chDijkstra.compute_shortest_path(src, dst);
        result = cchGraph.unpack_shortcuts(result);
        // Assume result.path is vector<NodeId>. Adjust to your actual type.
        const vector<NodeId> &actual_path = result.path;      // full expanded path
        const vector<NodeId> &shortcut_path = result.ch_path; // upward/downward CH hops
        const double cost = result.total_cost;                     // total cost (distance/time)

        // Basic sanity: if there is clearly no path, return non-zero (so server 500s)
        if (actual_path.empty())
        {
            cerr << "CH: no path found.\n";
            return 3;
        }

        // --- JSON to stdout ONLY ---
        cout << "{\"algo\":\"ch\",\"actual_path\":[";
        for (size_t i = 0; i < actual_path.size(); ++i)
        {
            if (i)
                cout << ',';
            cout << actual_path[i];
        }
        cout << "],\"shortcut_path\":[";
        for (size_t i = 0; i < shortcut_path.size(); ++i)
        {
            if (i)
                cout << ',';
            cout << shortcut_path[i];
        }
        cout << "],\"cost\":" << cost << "}\n";

        return 0;
    }
    // print or use result

    static void run_ch_single_benchmark()
    {
        cout << "running CH_graph" << endl;
        // Add code to run CCH benchmark
        vector<string> filepaths = {
            "./RoadNetworks/test.txt",
            // "./RoadNetworks/testcch.txt",
            // "./RoadNetworks/osm1.txt",
            // "./RoadNetworks/osm2.txt",
            // "./RoadNetworks/osm3.txt",
            // "./RoadNetworks/osm4.txt",
            // "./RoadNetworks/osm5.txt",
            // "./RoadNetworks/osm6.txt",
            // "./RoadNetworks/osm7.txt",
            // "./RoadNetworks/osm8.txt",
            // "./RoadNetworks/osm9.txt",
            // "./RoadNetworks/osm10.txt",
            // "./RoadNetworks/osm11.txt",
        };

        const string OUT_DIR = "output_files/1b";

        for (const auto &filepath : filepaths)
        {
            cout << "---------------------><><-----------------------" << endl;
            FileHandler fh;
            Graph graph = fh.read_file(filepath);
            cout << "Loaded file:" << filepath << "\n";

            CH ch(graph);
            ch.preprocess();

            CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
            CH_Dijkstra chDijkstra(chGraph);
            string line;
            while (true)
            {
                cout << "query> ";
                if (!getline(cin, line))
                    return; // EOF -> stop all
                if (line.empty())
                    continue;

                // parse "u v"
                istringstream iss(line);
                int u, v;
                iss >> u >> v;
                // cout << "Please enter: <src> <dst> (integers), or 'next', or 'q'\n";

                auto c0 = high_resolution_clock::now();
                CH_DijkstraResult cres = chDijkstra.compute_shortest_path(u, v);
                auto c1 = high_resolution_clock::now();

                cout << "cost: " << cres.total_cost;
            }
        }
    }

    static void run_ch_And_Dijkstra_Query(const string &map, NodeId src, NodeId dst)
    {
        // Map short names to actual file paths
        static const unordered_map<string, string> path_map = {
            {"test", "./RoadNetworks/test.txt"},
            {"testcch", "./RoadNetworks/testcch.txt"},
            {"osm1", "./RoadNetworks/osm1.txt"},
            {"osm2", "./RoadNetworks/osm2.txt"},
            {"osm3", "./RoadNetworks/osm3.txt"},
            {"osm4", "./RoadNetworks/osm4.txt"},
            {"osm5", "./RoadNetworks/osm5.txt"},
            {"osm6", "./RoadNetworks/osm6.txt"},
            {"osm7", "./RoadNetworks/osm7.txt"},
            {"osm8", "./RoadNetworks/osm8.txt"},
            {"osm9", "./RoadNetworks/osm9.txt"},
            {"osm10", "./RoadNetworks/osm10.txt"},
            {"osm11", "./RoadNetworks/osm11.txt"},
        };

        auto it = path_map.find(map);
        if (it == path_map.end())
        {
            cerr << "Unknown map choice: " << map << endl; // stderr
            return;                                                  // non-zero = error
        }

        const string filepath = it->second;
        cerr << "Running Dijkstra on " << filepath
                  << " src=" << src << " dst=" << dst << endl;

        FileHandler fh;
        Graph graph = fh.read_file(filepath);

        Dijkstra dijkstra(graph);
        auto result = dijkstra.compute_shortest_path(src, dst);
        // Assume result.path is vector<NodeId>. Adjust to your actual type.
        const auto &path = result.path;

        CH ch(graph);
        ch.preprocess();

        CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
        CH_Dijkstra chDijkstra(chGraph);

        CH_DijkstraResult ch_result = chDijkstra.compute_shortest_path(src, dst);
        ch_result = chGraph.unpack_shortcuts(ch_result);
        // Assume result.path is vector<NodeId>. Adjust to your actual type.
        // Your result fields — adjust names if your struct differs:
        const vector<NodeId> &actual_path = ch_result.path;      // full expanded path
        const vector<NodeId> &shortcut_path = ch_result.ch_path; // upward/downward CH hops
        const double cost = ch_result.total_cost;      

        // ---- STRICT JSON output to stdout ----
        cout << "{";

// --- Dijkstra section ---
cout << "\"djk\":{";
cout << "\"algo\":\"djk\",\"path\":[";
for (size_t i = 0; i < path.size(); i++) {
    if (i) cout << ",";
    cout << path[i];
}
cout << "]";
cout << ",\"cost\":" << result.total_cost; // adjust if your Dijkstra result has a cost
cout << "}";

// --- CH section ---
cout << ",\"ch\":{";
cout << "\"algo\":\"ch\",\"actual_path\":[";
for (size_t i = 0; i < actual_path.size(); i++) {
    if (i) cout << ",";
    cout << actual_path[i];
}
cout << "],\"shortcut_path\":[";
for (size_t i = 0; i < shortcut_path.size(); i++) {
    if (i) cout << ",";
    cout << shortcut_path[i];
}
cout << "],\"cost\":" << cost;
cout << "}";

cout << "}" << endl; // close outer object
    }
    // print or use result

};
