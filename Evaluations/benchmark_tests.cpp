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
#include <unordered_set>
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
void run_src_dst_benchmark_on_graph(Graph& graph, const string& output_csv_path) {
    const int n = graph.number_of_nodes();
    vector<pair<int, int>> query_pairs = generate_query_pairs(n);

    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    ofstream out(output_csv_path);
    out << boolalpha;
    out << "src,dst,d_time_us,d_cost,s_time_us,s_cost,nv_time_us,nv_cost,"
        << "d_pops,s_pops,nv_pops,"
        << "d_avg_pops_per_node,s_avg_pops_per_node,nv_avg_pops_per_node,"
        << "in_pert,in_pert_trans,out_pert,pert,matched\n";

    cout << "Running 100 src-dst queries..." << endl;
    int mismatch = 0;
    for (auto [src, dst] : query_pairs) {
        auto d_start = chrono::high_resolution_clock::now();
        SsspResult rd = dijkstra.compute_shortest_path(src, dst);
        auto d_end = chrono::high_resolution_clock::now();
        long long d_time_us = chrono::duration_cast<chrono::microseconds>(d_end - d_start).count();
        Cost dc = rd.total_cost;

        auto s_start = chrono::high_resolution_clock::now();
        SsspResult rs = spira.compute_shortest_path(src, dst);
        auto s_end = chrono::high_resolution_clock::now();
        long long s_time_us = chrono::duration_cast<chrono::microseconds>(s_end - s_start).count();
        Cost sc = rs.total_cost;

        auto n_start = chrono::high_resolution_clock::now();
        SsspResult rn = new_variant.compute_shortest_path(src, dst);
        auto n_end = chrono::high_resolution_clock::now();
        long long n_time_us = chrono::duration_cast<chrono::microseconds>(n_end - n_start).count();
        Cost nc = rn.total_cost;

        auto same_cost = [](Cost a, Cost b) {
            if (a < 0 || b < 0) return a == b;
            return fabs(a - b) <= 1e-9;
        };

        bool matched = same_cost(dc, sc) && same_cost(dc, nc);

        if (!matched) {
            cout << "Warning: cost mismatch for src=" << src << ", dst=" << dst << "\n";
            for(auto i = 0; i < rn.path.size(); i++){
                cout << rn.path[i] << " ";
            }
            mismatch++;
        }

        int in_pertinent_edges = 0;
        int in_pertinent_edges_transferred = 0;
        int out_pertinent_edges = 0;

        for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid) {
        if (rn.in_pertinent_edges[eid]) {
            in_pertinent_edges += 1;
            if(rn.in_pertinent_edges_extracted_in_forward_phase[eid]){
                in_pertinent_edges_transferred += 1;
            } 
        }
        else if (rn.out_pertinent_edges[eid]) {
            out_pertinent_edges += 1;
        }
    }
    int total_pertinent_edges = in_pertinent_edges + out_pertinent_edges;
    
        out << src << ',' << dst << ','
            << d_time_us << ',' << dc << ',' << s_time_us << ',' << sc << ',' << n_time_us << ',' << nc << ','
            << rd.number_of_pops << ',' << rs.number_of_pops << ',' << rn.number_of_pops << ','
            << rd.avg_pops_per_node << ',' << rs.avg_pops_per_node << ',' << rn.avg_pops_per_node << ','
            << in_pertinent_edges << ',' << in_pertinent_edges_transferred << ',' << out_pertinent_edges << ',' << total_pertinent_edges << ','
            << matched << '\n';
    }
    cout << "Number of Mismatched Queries: " << mismatch << endl;
    out.close();
}

// ---- Helper: Count node-wise mismatches between two SPTs ----
static int count_distance_mismatches(const std::vector<Cost>& a,
                                     const std::vector<Cost>& b)
{
    int mismatches = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::fabs(a[i] - b[i]) > 1e-9) // tolerance for floating-point comparison
            mismatches++;
    }
    return mismatches;
}

// ---- Helper: Compare SPT equality ----
static bool compare_spt_results(const SsspResult& a,
                                const SsspResult& b,
                                const std::string& nameA,
                                const std::string& nameB)
{
    if (a.distance.size() != b.distance.size())
        return false;

    for (size_t i = 0; i < a.distance.size(); ++i) {
        Cost da = a.distance[i];
        Cost db = b.distance[i];
        if ((da < INF_COST && db < INF_COST && std::fabs(da - db) > 1e-9) ||
            (da == INF_COST && db != INF_COST) ||
            (db == INF_COST && da != INF_COST))
        {
            cout << "Distance mismatch at node " << i << ": "
                 << nameA << " has " << da << ", "
                 << nameB << " has " << db << "\n";
            return false;
        }
    }
    return true;
}

// ---- Main Benchmark ----
void run_spt_benchmark_on_graph(Graph& graph, const std::string& output_csv_path)
{
    const int n = graph.number_of_nodes();
    const int num_sources = std::min(n, 100);

    // --- Select 100 unique random sources ---
    std::vector<int> sources;
    sources.reserve(num_sources);

    std::mt19937 rng(42); // fixed seed for reproducibility
    std::uniform_int_distribution<int> dist(0, n - 1);

    std::unordered_set<int> used;
    while (sources.size() < num_sources) {
        int src = dist(rng);
        if (used.insert(src).second)
            sources.push_back(src);
    }

    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    std::ofstream out(output_csv_path);
    out << "src,"
        << "d_time_us,s_time_us,nv_time_us,"
        << "d_pops,s_pops,nv_pops,"
        << "d_avg_pops_per_node,s_avg_pops_per_node,nv_avg_pops_per_node,"
        << "in_pert,in_pert_trans,out_pert,pert,"
        << "mismatch_count_djk_spi,mismatch_count_djk_new,"
        << "mismatches\n";

    int mismatch_runs = 0;

    std::cout << "Running " << num_sources << " SPT queries..." << std::endl;

    for (int src : sources) {
        // === Dijkstra ===
        auto d_start = std::chrono::high_resolution_clock::now();
        SsspResult rd = dijkstra.compute_shortest_path(src, -1);
        auto d_end = std::chrono::high_resolution_clock::now();
        long long d_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(d_end - d_start).count();

        // === Spira ===
        auto s_start = std::chrono::high_resolution_clock::now();
        SsspResult rs = spira.compute_shortest_path(src, -1);
        auto s_end = std::chrono::high_resolution_clock::now();
        long long s_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(s_end - s_start).count();

        // === New Variant ===
        auto n_start = std::chrono::high_resolution_clock::now();
        SsspResult rn = new_variant.compute_shortest_path(src, -1);
        auto n_end = std::chrono::high_resolution_clock::now();
        long long n_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(n_end - n_start).count();

        int in_pertinent_edges = 0;
        int out_pertinent_edges = 0;
        int in_pertinent_edges_transferred = 0;

        // cout << "in_pertinent_edges: " << rn.in_pertinent_edges.size() << endl;
        for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid) {
        if (rn.in_pertinent_edges[eid]) {
            in_pertinent_edges += 1;
            if(rn.in_pertinent_edges_extracted_in_forward_phase[eid]){
                in_pertinent_edges_transferred += 1;
            }
        }
        else if (rn.out_pertinent_edges[eid]) {
            out_pertinent_edges += 1;
        }
    }
    int total_pertinent_edges = in_pertinent_edges + out_pertinent_edges;
        // === Compare distances ===
        bool match_ds = compare_spt_results(rd, rs, "Dijkstra", "Spira");
        bool match_dn = compare_spt_results(rd, rn, "Dijkstra", "NewVariant");

        int mismatch_count_ds = count_distance_mismatches(rd.distance, rs.distance);
        int mismatch_count_dn = count_distance_mismatches(rd.distance, rn.distance);

        bool mismatches = !(match_ds && match_dn);
        if (mismatches)
            mismatch_runs++;

        // === Write one CSV row ===
        out << src << ','
            << d_time_us << ',' << s_time_us << ',' << n_time_us << ','
            << rd.number_of_pops << ',' << rs.number_of_pops << ',' << rn.number_of_pops << ','
            << rd.avg_pops_per_node << ',' << rs.avg_pops_per_node << ',' << rn.avg_pops_per_node << ','
            << in_pertinent_edges << ',' << in_pertinent_edges_transferred << ',' << out_pertinent_edges << ',' << total_pertinent_edges << ','
            << mismatch_count_ds << ',' << mismatch_count_dn << ','
            << std::boolalpha << mismatches << '\n';
    }

    out.close();

    // std::cout << "Benchmark complete.\n";
    std::cout << "Mismatches in " << mismatch_runs << " / " << num_sources << " runs.\n";
    // std::cout << "Results saved to " << output_csv_path << std::endl;
    std::cout << std::endl;
}

// ---- Wrapper to read and run ----
void process_sparse_graph_file(const string& filepath, WeightMode weight_mode, string& output_dir) {
    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file(filepath, weight_mode);
    graph.sort_all_neighbors();
    
    string output_dir1 = output_dir + "/src_dst_benchmark";
    create_directories(output_dir1); // make sure directory exists

    string filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    string output_csv = output_dir1 + "/" + filename + ".csv";
    // cout << "Running on " << filepath << " -> " << output_csv << endl;
    cout << "Running src-dst benchmark -> " << output_csv << endl;
    run_src_dst_benchmark_on_graph(graph, output_csv);

    string output_dir2 = output_dir + "/spt_benchmark";
    create_directories(output_dir2); // make sure directory exists
    cout << endl;
    filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    output_csv = output_dir2 + "/" + filename + ".csv";
    cout << "Running spt benchmark -> " << output_csv << endl;
    run_spt_benchmark_on_graph(graph, output_csv);

    // cout << endl;
}

// ---- Wrapper to read and run ----
void process_dense_graph_file(const string& filepath, WeightMode weight_mode, string& output_dir) {
    FileHandler fh;
    Graph graph = fh.read_dense_graph_file(filepath, weight_mode);
    graph.sort_all_neighbors();

    string output_dir1 = output_dir + "/src_dst_benchmark";
    create_directories(output_dir1); // make sure directory exists

    string filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    string output_csv = output_dir1 + "/" + filename + ".csv";

    // cout << "Running on " << filepath << " -> " << output_csv << endl;
    cout << "Running src-dst benchmark -> " << output_csv << endl;
    run_src_dst_benchmark_on_graph(graph, output_csv);

    string output_dir2 = output_dir + "/spt_benchmark";
    create_directories(output_dir2); // make sure directory exists

    filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    output_csv = output_dir2 + "/" + filename + ".csv";
    cout << endl;
    cout << "Running spt benchmark -> " << output_csv << endl;
    run_spt_benchmark_on_graph(graph, output_csv);
}

int run_benchmark_on_sparse_graphs() {
    string input_dir = "./Input_Data/SparseRoadNetworks";
    string output_dir_random = "output/sparse_networks/random_weights";
    string output_dir_exponential = "output/sparse_networks/exponential_weights";
    string output_dir_original = "output/sparse_networks/original_weights";

    cout << "starting benchmark on sparse graphs...\n";
    cout << endl;
    cout << endl;
    for (const auto& entry : directory_iterator(input_dir)) {
        if (entry.path().extension() == ".txt") {
            string filepath = entry.path().string();
            cout << "Processing file: " << filepath << endl;
            cout << endl;
            cout << "Processing with random weights: " << endl;
            // Case 1: random weights (true)
            process_sparse_graph_file(filepath, WeightMode::UniformRandomDistribution, output_dir_random);

            cout << "Processing with exponential weights: " << endl;
            // Case 2: exponential weights
            process_sparse_graph_file(filepath, WeightMode::Exponential, output_dir_exponential);

            cout << "Processing with original weights: " << endl;
            // Case 3: original weights 
            process_sparse_graph_file(filepath, WeightMode::Original, output_dir_original);
        }
    }

    cout << "Benchmarking completed for all sparse graphs with original, exponential and uniformly random weights.\n";
    cout << endl;

    cout << "==============================================================================================================================" << endl;

    cout << endl;

    return 0;
}

int run_benchmark_on_dense_graphs() {
    string input_dir = "./Input_Data/DenseNetworks";
    string output_dir_random = "output/DenseNetworks/random_weights";
    string output_dir_original = "output/DenseNetworks/original_weights";
    string output_dir_exponential = "output/DenseNetworks/exponential_weights";
    string output_dir_uniform = "output/DenseNetworks/uniform_random_weights";

    cout << "starting benchmark on dense graphs...\n";
    cout << endl;
    cout << endl;

    for (const auto& entry : directory_iterator(input_dir)) {
        if (entry.path().extension() == ".tsp") {
            string filepath = entry.path().string();
            cout << "Processing file: " << filepath << endl;
            cout << endl;

            cout << "Processing with original weights: " << endl;
            // Case 1: random weights (false), default is original weights
            process_dense_graph_file(filepath, WeightMode::Original, output_dir_original);

            cout << "Processing with random weights: " << endl;
            // Case 2: random weights (true), 
            process_dense_graph_file(filepath, WeightMode::UniformRandomDistribution, output_dir_random);

            // cout << "Processing with uniform weights: " << endl;
            // // Case 3: uniform weights (true), 
            // process_dense_graph_file(filepath, WeightMode::Uniform, output_dir_uniform);

            cout << "Processing with exponential weights: " << endl;
            // Case : Exponential weights (true), 
            process_dense_graph_file(filepath, WeightMode::Exponential, output_dir_exponential);
        }
    }

     cout << "Benchmarking completed for all dense graphs with original, uniform, exponential and uniformly random weights.\n";
    cout << endl;

    cout << "==============================================================================================================================" << endl;
    cout << endl;
    
    return 0;
}

int run_benchmark_on_exponential_size_sweep(int min_n = 100,
                                            int max_n = 3000,
                                            int num_sizes = 10,
                                            double lambda = 1.0,
                                            uint64_t base_seed = 4242,
                                            string base_output_dir = "output/exp_complete",
                                            bool symmetric_bidirectional = false)
{
    using namespace std;

    if (num_sizes <= 1) { cerr << "num_sizes must be >= 2\n"; return -1; }
    if (min_n <= 0 || max_n < min_n) { cerr << "invalid n range\n"; return -1; }

    int step = (max_n - min_n) / (num_sizes - 1);
    if (step <= 0) step = 1;

    string base_output_dir1 = base_output_dir + "/src_dst_benchmark";
    create_directories(base_output_dir1);

    string base_output_dir2 = base_output_dir + "/spt_benchmark";
    create_directories(base_output_dir2);

    FileHandler fh;

    for (int k = 0; k < num_sizes; ++k) {
        int n = min_n + k * step;
        if (n > max_n) n = max_n;

        uint64_t seed = base_seed + static_cast<uint64_t>(n) * 1000003ULL;

        cout << "\n=== Generating exponential graph: n=" << n
             << " (" << (k + 1) << "/" << num_sizes << ") ===\n";

        cout << endl;
        Graph graph = fh.generate_complete_exponential_graph(n, lambda, seed, symmetric_bidirectional);
        graph.sort_all_neighbors();

        // Save CSV directly in base_output_dir
        ostringstream fname;
        fname << base_output_dir1 << "/n" << n << ".csv";

        // for spt
        ostringstream fname2;
        fname2 << base_output_dir2 << "/n" << n << ".csv";

        // cout << "Running benchmark -> " << fname.str() << endl;
        cout << "Running src-dst benchmark -> " << fname.str() << endl;
        run_src_dst_benchmark_on_graph(graph, fname.str());
        cout << endl;
        cout << "Running spt benchmark -> " << fname2.str() << endl;
        run_spt_benchmark_on_graph(graph, fname2.str());
    }

    cout << "\nCompleted exponential size sweep: "
         << num_sizes << " graphs (" << min_n << "–" << max_n << " nodes)\n";
    cout << endl;
    cout << "==============================================================================================================================" << endl;
    cout << endl;

    return 0;
}
