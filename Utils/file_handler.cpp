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

Graph FileHandler::read_dense_graph_file(const string &filePath, bool use_random_weights, bool use_uniform_weights)
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

            if (use_uniform_weights) {
                edge.cost = 1;
            }
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

Graph FileHandler::generate_complete_exponential_graph(int n,
                                                       double lambda,
                                                       bool use_seed,
                                                  uint64_t seed)
{
    using namespace std;

    if (n <= 0) {
        cerr << "Error: number of nodes must be > 0\n";
        return Graph{};
    }

    mt19937_64 gen(use_seed ? seed : random_device{}());
    exponential_distribution<Cost> expdist(lambda);

    Graph graph(n);

    // Create nodes (just assign IDs if you don’t have lat/long here)
    for (int i = 0; i < n; ++i) {
        Node node;
        node.id = i;
        node.latitude = 0.0;   // or skip if unused
        node.longitude = 0.0;
        graph.set_node(node);
    }

    // Create bidirectional edges with exponential weights
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            Edge edge, reverse_edge;
            edge.src = i;
            edge.trg = j;
            reverse_edge.src = j;
            reverse_edge.trg = i;

            Cost w = expdist(gen);
            edge.cost = w;
            reverse_edge.cost = w;

            graph.set_edge(edge, reverse_edge);
        }
    }

    return graph;
}

