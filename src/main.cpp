#include <iostream>
#include "file_handler.hpp"
#include <dijkstra.hpp>
#include <spira.hpp>
#include <new_variant.hpp>
#include <chrono>
#include <set>
#include <random>
#include "basic_test.cpp"
#include "benchmark_tests.hpp"
#include "visualizer.hpp"

using namespace std;
using namespace WebAPP;


int main(int argc, char* argv[])
{
    if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <command> [options]\n";
    cerr << "\nCommands:\n";
    // cerr << "  test                                  Run basic tests\n";
    // cerr << "  benchmark_sparse                      Run benchmarks on sparse graphs\n";
    // cerr << "  benchmark_dense                       Run benchmarks on dense graphs\n";
    // cerr << "  benchmark_exponential_sweep           Run benchmarks on exponential size sweep\n";
    cerr << "  visualize <file> <algo> <src> <dst> <weightMode> <graphType>\n";
    cerr << "                                        Visualize shortest path from file\n";
    // return 1;
}

// Visualization mode
if (argc > 1 &&string(argv[1]) == "visualize") {
    if (argc < 8) {
        cerr << "Usage: " << argv[0]
             << " visualize <file> <algo> <src> <dst> <weightMode> <graphType>\n";
        cerr << "Example:\n";
        cerr << "  " << argv[0]
             << " visualize osm1 Dijkstra 0 500 original sparse\n";
        // return 1;
    }

    string file = argv[2];
    string algorithm = argv[3];
    int src = stoi(argv[4]);
    int dst = stoi(argv[5]);
    string edgeWeightMode = argv[6];
    string graphType = argv[7];

    WeightMode weightMode;

    if (edgeWeightMode == "Original") {
        weightMode = WeightMode::Original;
    } else if (edgeWeightMode == "Uniform") {
        weightMode = WeightMode::Uniform;
    } else if (edgeWeightMode == "Exponential") {
        weightMode = WeightMode::Exponential;
    } else if (edgeWeightMode == "Random") {
        weightMode = WeightMode::UniformRandomDistribution;
    } else {
        cerr << "Unknown weight mode: " << edgeWeightMode << endl;
        return 1;
    }

    WebAPP::visualize_algorithm(file, algorithm, src, dst, weightMode, graphType);
    return 0;
}
    
    // To do: 
    // add a interative while loop. provide option 

   
    BenchmarkTests::run_benchmark_on_sparse_graphs();
    BenchmarkTests::run_benchmark_dense_graph_osm_edges();
    BenchmarkTests::run_benchmark_on_dense_graphs();
    BenchmarkTests::run_benchmark_on_exponential_size_sweep();


}
