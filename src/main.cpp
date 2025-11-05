#include <iostream>
#include "file_handler.hpp"
#include <dijkstra.hpp>
#include <spira.hpp>
#include <new_variant.hpp>
#include <chrono>
#include <set>
#include <random>
#include <basic_test.cpp>

using namespace std;

vector<pair<int,int>> generate_query_pairs(int n) {
    
     // ----- Generate 100 unique (src, dst) pairs -----
    mt19937 rng(62); // fixed seed for reproducibility
    uniform_int_distribution<int> dist(0, n - 1);
    
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

int main()
{
    // run_all_algorithms();
    FileHandler fh;
    Graph graph = fh.read_file("./RoadNetworks/osm5.txt");

    // If Spira requires sorted adjacency lists, do it once up front.
    graph.sort_all_neighbors();

  const int n = graph.number_of_nodes();

    vector<pair<int,int>> query_pairs = generate_query_pairs(n);
  // Build algorithms
    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    // ----- Containers for reporting -----
    struct Mismatch {
        int src, dst;
        long long d_cost;
        long long s_cost;
        long long nv_cost;
    };
    std::vector<Mismatch> mismatches;

    std::vector<std::pair<int,int>> no_path_dijkstra;
    std::vector<std::pair<int,int>> no_path_spira;
    std::vector<std::pair<int,int>> no_path_newvar;

    std::cout << std::boolalpha; // print true/false
    std::cout << "src,dst,d_time_us,d_cost,s_time_us,s_cost,nv_time_us,nv_cost,matched\n";

    // ----- Run all three algorithms on each pair -----
   for (auto [src, dst] : query_pairs) {
    // Dijkstra
    auto d_start = std::chrono::high_resolution_clock::now();
    DijkstraResult rd = dijkstra.compute_shortest_path(src, dst);
    auto d_end = std::chrono::high_resolution_clock::now();
    long long d_time_us = std::chrono::duration_cast<std::chrono::microseconds>(d_end - d_start).count();
    Cost dc = rd.total_cost;

    // Spira
    auto s_start = std::chrono::high_resolution_clock::now();
    DijkstraResult rs = spira.compute_shortest_path(src, dst);
    auto s_end = std::chrono::high_resolution_clock::now();
    long long s_time_us = std::chrono::duration_cast<std::chrono::microseconds>(s_end - s_start).count();
    Cost sc = rs.total_cost;

    // NewVariant
    auto n_start = std::chrono::high_resolution_clock::now();
    DijkstraResult rn = new_variant.compute_shortest_path(src, dst);
    auto n_end = std::chrono::high_resolution_clock::now();
    long long n_time_us = std::chrono::duration_cast<std::chrono::microseconds>(n_end - n_start).count();
    Cost nc = rn.total_cost;

    // equality for costs: exact match for -1, epsilon for valid doubles
    auto same_cost = [](Cost a, Cost b) {
        if (a < 0 || b < 0) return a == b;              // -1 must match exactly
        return std::fabs(a - b) <= 1e-9;                // epsilon for floating point
    };
    bool matched = same_cost(dc, sc) && same_cost(dc, nc);

    // optional: keep your no-path trackers
    if (dc < 0) no_path_dijkstra.emplace_back(src, dst);
    if (sc < 0) no_path_spira.emplace_back(src, dst);
    if (nc < 0) no_path_newvar.emplace_back(src, dst);

    // CSV log line
    std::cout << src << ',' << dst << ','
              << d_time_us << ',' << dc << ','
              << s_time_us << ',' << sc << ','
              << n_time_us << ',' << nc << ','
              << matched << '\n';
}

    // ----- Print results -----
    cout << "=== Summary over 100 queries ===\n\n";

    auto print_pairs = [&](const char* title, const std::vector<std::pair<int,int>>& v){
        cout << title << " (" << v.size() << "):\n";
        if (v.empty()) { cout << "  (none)\n\n"; return; }
        int count = 0;
        for (auto [s, t] : v) {
            cout << "  (" << s << ", " << t << ")";
            if (++count % 8 == 0) cout << '\n'; // wrap a bit
        }
        if (count % 8 != 0) cout << '\n';
        cout << '\n';
    };

    // Pairs where *that* algorithm returned -1 (no path)
    print_pairs("No-path pairs for Dijkstra", no_path_dijkstra);
    print_pairs("No-path pairs for Spira",    no_path_spira);
    print_pairs("No-path pairs for NewVariant", no_path_newvar);

    // Mismatches: show costs from all three
    cout << "Mismatched total_cost across algorithms (" << mismatches.size() << "):\n";
    if (mismatches.empty()) {
        cout << "  (none)\n";
    } else {
        for (const auto& m : mismatches) {
            cout << "  src=" << m.src << " dst=" << m.dst
                 << " | Dijkstra=" << m.d_cost
                 << " Spira=" << m.s_cost
                 << " NewVariant=" << m.nv_cost << '\n';
        }
    }
    cout << endl;
}