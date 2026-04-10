#pragma once
#include <string>
#include "data_structures/graph.hpp"

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
        Graph read_file(const string& file_path);
        
        //Prepares document for KaHIP Node Ordering
        void write_to_metis_format(const Graph& graph, const string& file_path);

        // Reads KaHIP output
        Ordering read_kahip_output(const string& file_path, int num_nodes);
};