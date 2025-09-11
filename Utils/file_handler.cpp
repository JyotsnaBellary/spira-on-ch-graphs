#include "file_handler.hpp"

#include <fstream>
#include <iostream>
#include <random>

Graph FileHandler::read_file(const string &filePath)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<Cost> wdist(0.0, 1.0); // inclusive [0,1]

    ifstream inputFile(filePath);

    if (!inputFile.is_open())
    {
        cerr << "Error opening file: " << filePath << endl;
        return Graph{};
    }

    int n, m;
    inputFile >> n >> m;

    Graph graph(n);

    // Read nodes
    for (int i = 0; i < n; ++i)
    {
        Node node;
        inputFile >> node.id >> node.latitude >> node.longitude;
        // cout << node.id << endl << node.latitude << endl << node.longitude << endl;
        graph.set_node(node);
    }

    // Read edges
    for (int i = 0; i < m; ++i)
    {
        Edge edge;
        Edge reverse_edge;

        inputFile >> edge.src >> edge.trg;
        edge.cost = wdist(gen);
        // edge.shortcut = false;

        reverse_edge.src = edge.trg;
        reverse_edge.trg = edge.src;
        reverse_edge.cost = 1;
        // reverse_edge.shortcut = false;
        // cout << edge.src << edge.trg << edge.cost;
        graph.set_edge(edge, reverse_edge);
    }

    // Process the file content
    inputFile.close();
    return graph;
}