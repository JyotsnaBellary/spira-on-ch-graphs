

#include <iostream>
#include "benchmarks.hpp"
#include "webApp.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <benchmark_name>\n";
        return 1;
    }

    std::string arg = argv[1];
    if (arg == "ch_100") {
        Benchmarks::run_ch_benchmarks();
    }
    if (arg == "cch_100") {
        Benchmarks::run_cch_benchmarks();
    }
    else if (arg == "ch") {
        Benchmarks::run_ch_single_benchmark();
    }
    else if (arg == "cch") {
        Benchmarks::run_cch_benchmark();
    }
    else if (arg == "dj") {
        Benchmarks::run_Dijkstra_benchmark();
    }
    else if (arg == "appquery") {
        if (argc < 5) {
            std::cerr << "Usage: " << argv[0] << " appquery <map> <src> <dst>\n";
            return 1;
        }

        std::string map = argv[2];
        NodeId src = std::stoi(argv[3]);
        NodeId dst = std::stoi(argv[4]);

        WebAPP::run_Dijkstra_Query(map, src, dst);
    }
    else {
        std::cerr << "Unknown benchmark: " << arg << "\n";
        return 1;
    }
    // Benchmarks::run_Dijkstra_benchmark();


    // Benchmarks::run_cch_benchmark();
    return 0;


}