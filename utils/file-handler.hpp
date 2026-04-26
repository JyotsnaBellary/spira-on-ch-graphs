#pragma once
#include <string>
#include <graph.hpp>
#include <enums.hpp>

using namespace std;

struct Kahip_Output {
    vector<NodeId> left;
    vector<NodeId> right;
    vector<NodeId> separator;
};

// Header file for File Handler
class FileHandler {
    public:
        // Reads graph Input
        // Graph read_file(const string& file_path);
        
        //Prepares document for KaHIP Node Ordering
        void write_to_metis_format(const Graph& graph, const string& file_path);

        // Reads KaHIP output
        Ordering read_kahip_output(const string& file_path, int num_nodes);

        // Reads sparse graph Input
        Graph read_sparse_graph_file(const string& file_path, WeightMode weight_mode = WeightMode::Original);
        // Reads dense graph Input
        Graph read_dense_graph_file(const string& file_path, WeightMode weight_mode = WeightMode::Original);

        Graph generate_complete_exponential_graph(int n,
                                                       double lambda,
                                                       bool use_seed,
                                                       uint64_t seed);

        Graph generate_complete_uniform_random_graph(int n,
                                                       double lambda,
                                                       bool use_seed,
                                                       uint64_t seed);
};