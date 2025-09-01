#pragma once
#include <string>
#include "data_structures/graph.hpp"

using namespace std;

struct Kahip_Output {
    vector<NodeId> left;
    vector<NodeId> right;
    vector<NodeId> separator;
};

class FileHandler {
    public:
        Graph read_file(const string& file_path);
        void write_to_metis_format(const Graph& graph, const string& file_path);
        Ordering read_kahip_output(const string& file_path, int num_nodes);
};