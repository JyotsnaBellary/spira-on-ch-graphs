

#include <iostream>
#include "benchmarks.hpp"
#include "webApp.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <benchmark_name>\n";
        return 1;
    }

    string arg = argv[1];

    // Runs 100 CH Queries on all instances
    if (arg == "ch_100") {
        Benchmarks::run_ch_benchmarks();
    }

    // Runs 100 CCH Quesries on all instances
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
    else if (arg == "appquerydjk") {
        if (argc < 5) {
            cerr << "Usage: " << argv[0] << " appquery <map> <src> <dst>\n";
            return 1;
        }

        string map = argv[2];
        NodeId src = stoi(argv[3]);
        NodeId dst = stoi(argv[4]);

        WebAPP::run_Dijkstra_Query(map, src, dst);
    }
    else if (arg == "appquerych") {
        if (argc < 5) {
            cerr << "Usage: " << argv[0] << " appquery <map> <src> <dst>\n";
            return 1;
        }

        string map = argv[2];
        NodeId src = stoi(argv[3]);
        NodeId dst = stoi(argv[4]);

        WebAPP::run_CH_Dijkstra_Query(map, src, dst);
    }
    else if (arg == "appquerycch") {
        if (argc < 5) {
            cerr << "Usage: " << argv[0] << " appquery <map> <src> <dst>\n";
            return 1;
        }

        string map = argv[2];
        NodeId src = stoi(argv[3]);
        NodeId dst = stoi(argv[4]);

        WebAPP::run_CCH_Dijkstra_Query(map, src, dst);
    }
    else if (arg == "appquerydjkch") {
        if (argc < 5) {
            cerr << "Usage: " << argv[0] << " appquery <map> <src> <dst>\n";
            return 1;
        }

        string map = argv[2];
        NodeId src = stoi(argv[3]);
        NodeId dst = stoi(argv[4]);

        WebAPP::run_ch_And_Dijkstra_Query(map, src, dst);
    }
    else {
        cerr << "Unknown benchmark: " << arg << "\n";
        return 1;
    }
    // Benchmarks::run_Dijkstra_benchmark();


    // Benchmarks::run_cch_benchmark();
    return 0;


}