#pragma once
#include <graph.hpp>
#include <string>

class FileHandler {
    public:
        // Reads sparse graph Input
        Graph read_sparse_graph_file(const string& file_path, bool use_random_weights = false);

        // Reads dense graph Input
        Graph read_dense_graph_file(const string& file_path, bool use_random_weights = false);
};