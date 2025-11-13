#pragma once
#include <graph.hpp>
#include <string>

enum class WeightMode {
    Original,                  // keep weights from file
    UniformRandomDistribution, // random weights in [0, 1]
    Exponential,               // exponential(λ)
    Uniform                  // all weights = 1 
};

class FileHandler {
    public:
        // Reads sparse graph Input
        Graph read_sparse_graph_file(const string& file_path, WeightMode weight_mode = WeightMode::Original);
        // Reads dense graph Input
        Graph read_dense_graph_file(const string& file_path, WeightMode weight_mode = WeightMode::Original);

        Graph generate_complete_exponential_graph(int n,
                                                       double lambda,
                                                       bool use_seed,
                                                       uint64_t seed);
};