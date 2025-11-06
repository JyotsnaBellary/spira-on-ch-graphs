#include "file_handler.hpp"

#include <fstream>
#include <iostream>
#include <random>

Graph FileHandler::read_sparse_graph_file(const string &filePath, bool use_random_weights)
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
        edge.cost = 1;
        if (use_random_weights) {
            edge.cost = wdist(gen);
        }
        // edge.shortcut = false;

        reverse_edge.src = edge.trg;
        reverse_edge.trg = edge.src;
        reverse_edge.cost = edge.cost;
        // reverse_edge.shortcut = false;
        // cout << edge.src << edge.trg << edge.cost;
        graph.set_edge(edge, reverse_edge);
    }

    // Process the file content
    inputFile.close();
    return graph;
}

Graph FileHandler::read_dense_graph_file(const string &filePath, bool use_random_weights)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<Cost> wdist(0.0, 1.0); // inclusive [0,1]

    ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return Graph{};
    }

    string line;
    int n = 0;

    // --- Parse header ---
    while (getline(inputFile, line)) {
        if (line.find("DIMENSION") != string::npos) {
            n = stoi(line.substr(line.find(":") + 1));
        }
        else if (line.find("NODE_COORD_SECTION") != string::npos) {
            break;
        }
    }

    if (n == 0) {
        cerr << "Error: DIMENSION not found in " << filePath << endl;
        return Graph{};
    }

    Graph graph(n);
    vector<Node> nodes(n);

    // --- Read nodes ---
    for (int i = 0; i < n; ++i) {
        Node node;
        inputFile >> node.id >> node.latitude >> node.longitude;
        node.id = node.id - 1;
        nodes[i] = node;
        graph.set_node(node);
    }

    inputFile.close();

    // --- Build complete graph with Euclidean distances ---
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i+1; j < n; ++j) {
            if (i == j) continue;

            Edge edge;
            edge.src = nodes[i].id; // zero-based
            edge.trg = nodes[j].id;

            double dx = nodes[i].latitude - nodes[j].latitude;
            double dy = nodes[i].longitude - nodes[j].longitude;
            double dist = sqrt(dx * dx + dy * dy);

            edge.cost = use_random_weights ? wdist(gen) 
                                           : static_cast<Cost>(dist);

            Edge reverse_edge;
            reverse_edge.src = edge.trg;
            reverse_edge.trg = edge.src;
            reverse_edge.cost = edge.cost;

            graph.set_edge(edge, reverse_edge);
        }
    }
    // cout << "Finished reading dense graph from " << filePath << " with " 
    //      << graph.number_of_nodes() << " nodes and " 
    //      << graph.number_of_edges() << " edges." << endl;
    return graph;
}
