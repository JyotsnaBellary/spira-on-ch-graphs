// Test 1: as give in the paper using complete directed graphs. (they say with high probability, what does that even mean?)
    // report results for SPT and for shortest path between src / trg pairs
// Test 2: use sparse graphs. 
    // report results for SPT and for shortest path between src / trg pairs
// Test 3: Check other pace challenge instances to work with. 

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <set>
#include <random>
#include <file_handler.hpp>
#include <dijkstra.hpp>
#include <spira.hpp>
#include <new_variant.hpp>

using namespace std;
using namespace filesystem;

vector<pair<int,int>> generate_query_pairs(int n) {
    
     // ----- Generate 100 unique (src, dst) pairs -----
    mt19937 rng(62); // fixed seed for reproducibility
    uniform_int_distribution<int> dist(0, n - n/10);
    
    vector<pair<int,int>> query_pairs;
    query_pairs.reserve(100);
    set<long long> used; // encode (src,dst) as 64-bit to ensure uniqueness

    auto encode = [](int s, int t) -> long long {
        return (static_cast<long long>(static_cast<unsigned int>(s)) << 32)
             | static_cast<unsigned int>(t);
    };

    while (static_cast<int>(query_pairs.size()) < 100) {
        int s = dist(rng);
        int t = dist(rng);
        if (s == t) continue;
        long long key = encode(s, t);
        if (used.insert(key).second) {
            query_pairs.emplace_back(s, t);
        }
    }

    return query_pairs;
}

// ---- Benchmark helper ----
void run_benchmark_on_graph(Graph& graph, const string& output_csv_path) {
    const int n = graph.number_of_nodes();
    vector<pair<int, int>> query_pairs = generate_query_pairs(n);

    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    ofstream out(output_csv_path);
    out << boolalpha;
    out << "src,dst,d_time_us,d_cost,s_time_us,s_cost,nv_time_us,nv_cost,matched\n";

    for (auto [src, dst] : query_pairs) {
        // cout << "Running src=" << src << ", dst=" << dst << endl;
        auto d_start = chrono::high_resolution_clock::now();
        DijkstraResult rd = dijkstra.compute_shortest_path(src, dst);
        auto d_end = chrono::high_resolution_clock::now();
        long long d_time_us = chrono::duration_cast<chrono::microseconds>(d_end - d_start).count();
        Cost dc = rd.total_cost;
        // cout << "Dijkstra done." << endl;

        auto s_start = chrono::high_resolution_clock::now();
        DijkstraResult rs = spira.compute_shortest_path(src, dst);
        auto s_end = chrono::high_resolution_clock::now();
        long long s_time_us = chrono::duration_cast<chrono::microseconds>(s_end - s_start).count();
        Cost sc = rs.total_cost;
// cout << "Spira done." << endl;
        auto n_start = chrono::high_resolution_clock::now();
        DijkstraResult rn = new_variant.compute_shortest_path(src, dst);
        auto n_end = chrono::high_resolution_clock::now();
        long long n_time_us = chrono::duration_cast<chrono::microseconds>(n_end - n_start).count();
        Cost nc = rn.total_cost;
        // cout << "NewVariant done." << endl;
        // cout << "Completed src=" << src << ", dst=" << dst << endl;
        auto same_cost = [](Cost a, Cost b) {
            if (a < 0 || b < 0) return a == b;
            return fabs(a - b) <= 1e-9;
        };
        bool matched = same_cost(dc, sc) && same_cost(dc, nc);

        out << src << ',' << dst << ','
            << d_time_us << ',' << dc << ','
            << s_time_us << ',' << sc << ','
            << n_time_us << ',' << nc << ','
            << matched << '\n';
    }
    out.close();
}

// ---- Wrapper to read and run ----
void process_sparse_graph_file(const string& filepath, bool uniform_weights, const string& output_dir) {
    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file(filepath, uniform_weights);
    graph.sort_all_neighbors();

    create_directories(output_dir); // make sure directory exists

    string filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    string output_csv = output_dir + "/" + filename + ".csv";

    cout << "Running on " << filepath << " -> " << output_csv << endl;
    run_benchmark_on_graph(graph, output_csv);
}

// ---- Wrapper to read and run ----
void process_dense_graph_file(const string& filepath, bool uniform_weights, const string& output_dir) {
    FileHandler fh;
    Graph graph = fh.read_dense_graph_file(filepath, uniform_weights);
    graph.sort_all_neighbors();

    create_directories(output_dir); // make sure directory exists

    string filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    string output_csv = output_dir + "/" + filename + ".csv";

    cout << "Running on " << filepath << " -> " << output_csv << endl;
    run_benchmark_on_graph(graph, output_csv);
}

int run_benchmark_on_sparse_graphs() {
    string input_dir = "./RoadNetworks";
    string output_dir_random = "output/sparse_networks/random_weights";
    string output_dir_original = "output/sparse_networks/original_weights";

    for (const auto& entry : directory_iterator(input_dir)) {
        if (entry.path().extension() == ".txt") {
            string filepath = entry.path().string();

            // Case 1: random weights (true)
            process_sparse_graph_file(filepath, true, output_dir_random);

            // Case 2: random weights (false), default is uniform weights
            process_sparse_graph_file(filepath, false, output_dir_original);
        }
    }

    cout << "Benchmarking completed for all sparse graphs with uniform and random weights.\n";
    return 0;
}

int run_benchmark_on_dense_graphs() {
    string input_dir = "./Input_Data/DenseNetworks";
    string output_dir_random = "output/dense_networks/random_weights";
    string output_dir_uniform = "output/dense_networks/weight_one";

    for (const auto& entry : directory_iterator(input_dir)) {
        if (entry.path().extension() == ".tsp") {
            string filepath = entry.path().string();

            // Case 1: random weights (true)
            process_dense_graph_file(filepath, false, output_dir_uniform);
            
            // Case 2: random weights (false), default is uniform weights
            process_dense_graph_file(filepath, true, output_dir_random);
        }
    }

    cout << "Benchmarking completed for all sparse graphs with uniform and random weights.\n";
    return 0;
}