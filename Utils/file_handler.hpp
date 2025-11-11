#pragma once
#include <graph.hpp>
#include <string>

class FileHandler {
    public:
        // Reads sparse graph Input
        Graph read_sparse_graph_file(const string& file_path, bool use_random_weights = false);

        // Reads dense graph Input
        Graph read_dense_graph_file(const string& file_path, bool use_random_weights = false, bool use_uniform_weights = false);

        Graph generate_complete_exponential_graph(int n,
                                                       double lambda,
                                                       bool use_seed,
                                                       uint64_t seed);
};