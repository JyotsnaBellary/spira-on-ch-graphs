#include <data_structures/graph.hpp>
#include <io/file_handler.hpp>
#include <algorithms/dijkstra.hpp>
#include <algorithms/CH.hpp>
#include <data_structures/ch_graph.hpp>
#include <algorithms/ch_dijkstra.hpp>
#include <random>
#include <unordered_set>
#include <fstream>
#include <algorithms/CCH.hpp>

using namespace std;
using namespace std::chrono;

struct QueryRow
{
    int src;
    int dst;
    long long dijk_time_ms;
    int dijk_pops;
    long long dijk_dist;
    long long ch_time_ms;
    int ch_pops;
    long long ch_dist;
    // long long cch_time_ms;
    // int cch_pops;
    // long long cch_dist;
    bool equal;
};

class Benchmarks
{
public:
    static void run_Dijkstra_benchmark(Graph graph)
    {
        // Add code to run Dijkstra benchmark
        Dijkstra dijkstra(graph);
        NodeId src, dst;
        DijkstraResult normal_result = dijkstra.compute_shortest_path(src, dst);
        cout << "Dijkstra Result:" << endl;
        cout << "Total cost: " << normal_result.total_cost << endl;
        for (auto &node : normal_result.path)
        {
            cout << node << " ";
        }
        cout << endl;

        // collect results
    }

    static void run_ch_benchmark()
    {
        cout << "running CH_graph" << endl;
        // Add code to run CCH benchmark
        std::vector<std::string> filepaths = {
            // "./RoadNetworks/test.txt",
            // "./RoadNetworks/testcch.txt",
            "./RoadNetworks/osm1.txt",
            "./RoadNetworks/osm2.txt",
            "./RoadNetworks/osm3.txt",
            "./RoadNetworks/osm4.txt",
            "./RoadNetworks/osm5.txt",
            "./RoadNetworks/osm6.txt",
            "./RoadNetworks/osm7.txt",
            "./RoadNetworks/osm8.txt",
            "./RoadNetworks/osm9.txt",
            "./RoadNetworks/osm10.txt",
            "./RoadNetworks/osm11.txt",
        };

        const std::string OUT_DIR = "output_files/1b";

        for (const auto &filepath : filepaths)
        {
            cout << "---------------------><><-----------------------" << endl;
            FileHandler fh;
            Graph graph = fh.read_file(filepath);
            std::cout << "Loaded file:" << filepath << "\n";

            CH ch(graph);
            ch.preprocess();

        //     CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
        //     CH_Dijkstra chDijkstra(chGraph);
        //     std::string line;
        // while (true) {
        //     std::cout << "query> ";
        //     if (!std::getline(std::cin, line)) return;          // EOF -> stop all
        //     if (line.empty()) continue;

            

        //     // parse "u v"
        //     std::istringstream iss(line);
        //     int u, v;
        //     iss >> u >> v;
        //         std::cout << "Please enter: <src> <dst> (integers), or 'next', or 'q'\n";
            

        //     auto c0 = high_resolution_clock::now();
        //         CH_DijkstraResult cres = chDijkstra.compute_shortest_path(u, v);
        //         auto c1 = high_resolution_clock::now();

        //         cout << "cost: "<< cres.total_cost;
        // }

        }
        
    }

    static void run_cch_benchmark()
    {
        cout << "running CCH_graph" << endl;
        // Add code to run CCH benchmark
        std::vector<std::string> filepaths = {
            // "./RoadNetworks/test.txt",
            // "./RoadNetworks/testcch.txt",
            "./RoadNetworks/osm1.txt",
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

        const std::string OUT_DIR = "output_files/1b";

        for (const auto &filepath : filepaths)
        {
            cout << "---------------------><><-----------------------" << endl;
            FileHandler fh;
            Graph graph = fh.read_file(filepath);
            cout << "Loaded files" << endl;
            std::cout << "Loaded file:" << filepath << "\n";
            cout << "Loaded files" << endl;

            CCH cch(graph);
            CH_Graph cch_graph = cch.preprocess();

            // CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
            CH_Dijkstra chDijkstra(cch_graph);
            std::string line;
        while (true) {
            std::cout << "query> ";
            if (!std::getline(std::cin, line)) return;          // EOF -> stop all
            if (line.empty()) continue;

            

            // parse "u v"
            std::istringstream iss(line);
            int u, v;
            iss >> u >> v;
            // if (!(iss >> u >> v)) {
                std::cout << "Please enter: <src> <dst> (integers), or 'next', or 'q'\n";
            //     continue;
            // }
            // if (u < 0 || v < 0 || u >= (int)graph.num_nodes() || v >= (int)graph.num_nodes()) {
            //     std::cout << "IDs out of range. Valid: 0.." << (graph.num_nodes()-1) << "\n";
            //     continue;
            // }

            // auto t0 = high_resolution_clock::now();

            // Adjust this call to match your CH_Dijkstra API:
            // Option A: returns (distance, path)

            auto c0 = high_resolution_clock::now();
                CH_DijkstraResult cres = chDijkstra.compute_shortest_path(u, v);
                auto c1 = high_resolution_clock::now();

                cout << "cost: "<< cres.total_cost;
        }

        }
        
    }

    // static void run_cch_benchmark()
    // {
    //     // Add code to run CCH benchmark
    //     std::vector<std::string> filepaths = {
    //         // "./RoadNetworks/test.txt",
    //         // "./RoadNetworks/testcch.txt",
    //         // "./RoadNetworks/osm1.txt",
    //         // "./RoadNetworks/osm2.txt",
    //         // "./RoadNetworks/osm3.txt",
    //         // "./RoadNetworks/osm4.txt",
    //         "./RoadNetworks/osm5.txt",
    //         // "./RoadNetworks/osm6.txt",
    //         // "./RoadNetworks/osm7.txt",
    //         // "./RoadNetworks/osm8.txt",
    //         // "./RoadNetworks/osm9.txt",
    //         // "./RoadNetworks/osm10.txt",
    //         // "./RoadNetworks/osm11.txt",
    //     };

    //     const std::string OUT_DIR = "output_files/1b";

    //     for (const auto &filepath : filepaths)
    //     {
    //         cout << "--------------------------------------------" << endl;
    //         FileHandler fh;
    //         Graph graph = fh.read_file(filepath);
    //         std::cout << "Loaded " << filepath << "\n";

    //         CCH cch(graph);
    //         CH_Graph cch_graph = cch.preprocess();

    //         CH_Dijkstra ch_dijkstra(cch_graph);
    //     }
        
    // }

    static void run_single_benchmark(const string &name)
    {
        // Add code to run a single benchmark by name
    }

    static inline long long to_ms(auto start, auto stop)
    {
        return duration_cast<milliseconds>(stop - start).count();
    }

    static inline std::vector<std::pair<int, int>>
    make_unique_random_pairs(int n, size_t count, uint64_t seed)
    {
        std::vector<std::pair<int, int>> pairs;
        pairs.reserve(count);
        if (n <= 1)
            return pairs;

        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<int> pick(0, n - 1);
        std::unordered_set<uint64_t> seen;
        seen.reserve(count * 2 + 7);

        auto key = [](int a, int b) -> uint64_t
        {
            return (uint64_t(uint32_t(a)) << 32) | uint64_t(uint32_t(b));
        };

        while (pairs.size() < count)
        {
            int s = pick(rng);
            int t = pick(rng);
            if (s == t)
                continue;
            uint64_t k = key(s, t);
            if (seen.insert(k).second)
                pairs.emplace_back(s, t);
        }
        return pairs;
    }

    static inline std::string basename_no_ext(const std::string &path)
    {
        std::filesystem::path p(path);
        return p.stem().string(); // e.g., "osm1"
    }

    static inline
    void write_csv(const std::string &out_dir,
                               const std::string &base_name,
                               const std::vector<QueryRow> &rows)
    {
        std::filesystem::create_directories(out_dir);
        std::string out_path = (std::filesystem::path(out_dir) / (base_name + ".csv")).string();
        std::ofstream out(out_path, std::ios::trunc);
        if (!out)
        {
            std::cerr << "Failed to open " << out_path << " for writing\n";
            return;
        }

        // header
        out << "src,dst,dijkstra_time_ms,priority_queue,dijkstra_distance,cch_time_ms,priority_queue,cch_distance,identical\n";
        out << "src,dst,dijkstra_time_ms,priority_queue,dijkstra_distance,ch_time_ms,priority_queue,ch_distance,identical\n";
        // out << "src,dst,dijkstra_time_ms,priority_queue,dijkstra_distance,ch_time_ms,priority_queue,ch_distance,ch_time_ms,priority_queue,ch_distance,identical\n";
        for (const auto &r : rows)
        {
            out << r.src << ','
                << r.dst << ','
                << r.dijk_time_ms << ','
                << r.dijk_pops << ','
                << r.dijk_dist << ','
                << r.ch_time_ms << ','
                << r.ch_pops << ','
                << r.ch_dist << ','
                // << r.cch_time_ms << ','
                // << r.cch_pops << ','
                // << r.cch_dist << ','
                << (r.equal ? "true" : "false") << ','
                << "" // priority_queue column empty for now
                << '\n';
        }
        out.close();
        std::cout << "Wrote " << rows.size() << " rows to " << out_path << "\n";
    }

    static void run_all_benchmarks()
    {
        std::vector<std::string> filepaths = {
            "./RoadNetworks/osm1.txt",
            "./RoadNetworks/osm2.txt",
            "./RoadNetworks/osm3.txt",
            "./RoadNetworks/osm4.txt",
            "./RoadNetworks/osm5.txt",
            "./RoadNetworks/osm6.txt",
            "./RoadNetworks/osm7.txt",
            "./RoadNetworks/osm8.txt",
            "./RoadNetworks/osm9.txt",
            "./RoadNetworks/osm10.txt",
            "./RoadNetworks/osm11.txt",
        };

        constexpr size_t NUM_QUERIES = 100;
        const std::string OUT_DIR = "output_files/1b-new";

        for (const auto &filepath : filepaths)
        {
            FileHandler fh;
            Graph ch_graph;
            Graph cch_graph;
            Graph graph = ch_graph = cch_graph = fh.read_file(filepath);
            const int n = graph.num_nodes();
            if (n <= 1)
            {
                std::cerr << "Skipping " << filepath << " (n=" << n << ")\n";
                continue;
            }
            std::cout << "Loaded " << filepath
                      << " with n=" << n << ", m=" << graph.num_edges() << "\n";

            // Baseline Dijkstra on original graph
            Dijkstra dijkstra(graph);

            // Build CH once per graph
            auto t0 = high_resolution_clock::now();
            CH ch(ch_graph);
            ch.preprocess(); // do the heavy work once
            auto t1 = high_resolution_clock::now();
            std::cout << "CH preprocess took " << to_ms(t0, t1) << " ms\n";

            // Build CH graph view (once), and the query engine
            CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
            CH_Dijkstra chDijkstra(chGraph);

            // // Build CCH once per graph
            // auto cc0 = high_resolution_clock::now();
            // CCH cch(cch_graph);
            // CH_Graph cchGraph = cch.preprocess(); // do the heavy work once
            // auto cc1 = high_resolution_clock::now();
            // std::cout << "CCH preprocess took " << to_ms(cc0, cc1) << " ms\n";

            // // Build CH graph view (once), and the query engine
            // // CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order());
            // CH_Dijkstra cchDijkstra(cchGraph);

           
            // Make unique queries in [0..n-1]
            auto pairs = make_unique_random_pairs(n, NUM_QUERIES, /*seed=*/0xC0FFEEULL);

            std::vector<QueryRow> rows;
            rows.reserve(pairs.size());

            // Run queries
            for (auto [src, dst] : pairs)
            {
                // Dijkstra
                auto d0 = high_resolution_clock::now();
                DijkstraResult dres = dijkstra.compute_shortest_path(src, dst);
                auto d1 = high_resolution_clock::now();

                // CH-Dijkstra
                auto c0 = high_resolution_clock::now();
                CH_DijkstraResult cres = chDijkstra.compute_shortest_path(src, dst);
                auto c1 = high_resolution_clock::now();

                // CH-Dijkstra
                // auto cc0 = high_resolution_clock::now();
                // CH_DijkstraResult ccres = cchDijkstra.compute_shortest_path(src, dst);
                // auto cc1 = high_resolution_clock::now();

                long long d_ms = to_ms(d0, d1);
                long long c_ms = to_ms(c0, c1);
                // long long cc_ms = to_ms(cc0, cc1);

                int d_pops = dres.number_of_pops;
                int c_pops = cres.number_of_pops;
                // int cc_pops = ccres.number_of_pops;

                // If your result types differ, adapt these field names:
                long long d_dist = dres.total_cost;
                long long c_dist = cres.total_cost;
                // long long cc_dist = ccres.total_cost;

                // bool equal = (d_dist == cc_dist);
                bool equal = (d_dist == c_dist);
                // bool equal = (d_dist == c_dist == cc_dist);
                if (!equal)
                { 
                    if (d_dist != c_dist)
                    // Optional debugging print
                    std::cerr << "[DIFF] " << src << "->" << dst
                              << " dijkstra=" << d_dist
                              << " ch=" << c_dist << "\n";

                    // if(d_dist != c_dist)
                    // // if(d_dist != cc_dist)
                    // // Optional debugging print
                    // std::cerr << "[DIFF] " << src << "->" << dst
                    //           << " dijkstra=" << d_dist
                    //           << " cch=" << cc_dist << "\n";
                }

                rows.push_back(QueryRow{
                    // src, dst, d_ms, d_pops, d_dist, c_ms, c_pops, c_dist, cc_ms, cc_pops, cc_dist, equal});
                    src, dst, d_ms, d_pops, d_dist, c_ms, c_pops, c_dist, equal});
                    // src, dst, d_ms, d_pops, d_dist, cc_ms, cc_pops, cc_dist, equal});
            }

            // Write CSV
            write_csv(OUT_DIR, basename_no_ext(filepath), rows);
        }
    }
};
