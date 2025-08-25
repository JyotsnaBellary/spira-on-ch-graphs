#include "io/file_handler.hpp"
#include <fstream>
#include <iostream>

Graph FileHandler::read_file(const string& filePath) {
    ifstream inputFile(filePath);


    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return Graph{};
    }

    int n, m;
    inputFile >> n >> m;

    Graph graph(n);

    //Read nodes
    for (int i = 0; i < n; ++i) {
        Node node;
        inputFile >> node.id >> node.lat >> node.lon;
        graph.set_node(node.id, node);
        // cout << "Node ID: " << node.id << ", Lat: " << node.lat << ", Lon: " << node.lon << endl;
    }

    // Read edges
    // EdgeId edgeId = 0;
    for (int i = 0; i < m; ++i) {
        Edge edge;
        Edge reverse_edge;

        inputFile >> edge.src >> edge.trg;
        edge.cost = 1;
        edge.shortcut = false;
        // graph.set_edge(edge);

        reverse_edge.src = edge.trg;
        reverse_edge.trg = edge.src;
        reverse_edge.cost = 1;
        reverse_edge.shortcut = false;
        graph.set_edge(edge, reverse_edge);
    }
    // Process the file content
    inputFile.close();
    return graph;
}

// void FileHandler::export_to_CSV(Graph& graph, const string& nodesCSV, const string& edgesCSV, const vector<EdgeId>& shortest_path_edges) {
//     ofstream nodesFile(nodesCSV);
//     ofstream edgesFile(edgesCSV);

//     // Write nodes and edges to their respective files
//     nodesFile.close();
//     edgesFile.close();
// }
