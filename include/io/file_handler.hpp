#pragma once
#include <string>
#include "data_structures/graph.hpp"

using namespace std;

class FileHandler {
    public:
        Graph read_file(const string& file_path);
        // void export_to_CSV(const Graph& graph, const string& nodesCSV, const string& edgesCSV, const vector<EdgeId>& shortest_path_edges);

};