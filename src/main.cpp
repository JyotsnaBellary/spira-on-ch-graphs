

#include <iostream>
#include "benchmarks.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <benchmark_name>\n";
        return 1;
    }

    std::string arg = argv[1];
    if (arg == "ch_100") {
        Benchmarks::run_all_benchmarks();
    }
    if (arg == "ch") {
        Benchmarks::run_ch_benchmark();
    }
    if (arg == "cch") {
        Benchmarks::run_cch_benchmark();
    }
    else {
        std::cerr << "Unknown benchmark: " << arg << "\n";
        return 1;
    }

    // Benchmarks::run_cch_benchmark();
    return 0;
}