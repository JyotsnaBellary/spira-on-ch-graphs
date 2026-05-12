#include <file-handler.hpp>
#include <fstream>
#include <iostream>
#include <random>

// Graph FileHandler::read_file(const string &filePath)
// {
//     ifstream inputFile(filePath);

//     if (!inputFile.is_open())
//     {
//         cerr << "Error opening file: " << filePath << endl;
//         return Graph{};
//     }

//     int n, m;
//     inputFile >> n >> m;

//     Graph graph(n);

//     // Read nodes
//     for (int i = 0; i < n; ++i)
//     {
//         Node node;
//         inputFile >> node.id >> node.latitude >> node.longitude;
//         graph.set_node(node.id, node);
//     }

//     // Read edges
//     for (int i = 0; i < m; ++i)
//     {
//         Edge edge;
//         Edge reverse_edge;

//         inputFile >> edge.src >> edge.trg;
//         edge.cost = 1;
//         edge.shortcut = false;

//         reverse_edge.src = edge.trg;
//         reverse_edge.trg = edge.src;
//         reverse_edge.cost = 1;
//         reverse_edge.shortcut = false;
//         graph.set_edge(edge, reverse_edge);
//     }

//     // Process the file content
//     inputFile.close();
//     return graph;
// }


Graph FileHandler::read_sparse_graph_file(const string &filePath, WeightMode weight_mode)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<Cost> uniform_dist(0.0, 1.0); // inclusive [0,1]
    exponential_distribution<Cost> exp_dist(1.0); // λ = 1.0 (mean = 1)


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
        graph.set_node(node.id, node);
    }

    // Read edges
    for (int i = 0; i < m; ++i)
    {
        Edge edge;
        Edge reverse_edge;

        inputFile >> edge.src >> edge.trg;
        switch (weight_mode)
        {
            case WeightMode::Original:
            {
                // edge.cost = 1.0; // or read from file if costs exist

                // take euclidean distance instead of 1 and run again
                const Node &u = graph.get_node(edge.src);
                const Node &v = graph.get_node(edge.trg);
                double dx = u.latitude - v.latitude;
                double dy = u.longitude - v.longitude;
                edge.cost = sqrt(dx * dx + dy * dy);

                break;
        }
        case WeightMode::Uniform:
                edge.cost = 1.0;
                break;
            case WeightMode::UniformRandomDistribution:
                edge.cost = uniform_dist(gen);
                break;
            case WeightMode::Exponential:
                edge.cost = exp_dist(gen);
                break;
            default:
                edge.cost = 1.0;
                break;
        }

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

Graph FileHandler::read_dense_graph_file(const string &filePath, WeightMode weight_mode)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<Cost> uniform_dist(0.0, 1.0); // inclusive [0,1]
    exponential_distribution<Cost> exp_dist(1.0); // λ = 1.0 (mean = 1)

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
        graph.set_node( node.id, node);
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

            switch (weight_mode)
            {
                case WeightMode::Original:
                    edge.cost = static_cast<Cost>(dist);
                    break;
                case WeightMode::UniformRandomDistribution:
                    edge.cost = uniform_dist(gen);
                    break;
                case WeightMode::Exponential:
                    edge.cost = exp_dist(gen);
                    break;
                case WeightMode::Uniform:
                    edge.cost = 1;
                    break;
                default:
                    edge.cost = static_cast<Cost>(dist);
                    break;
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

    // Create nodes 
    for (int i = 0; i < n; ++i) {
        Node node;
        node.id = i;
        node.latitude = 0.0;   
        node.longitude = 0.0;
        graph.set_node(node.id, node);
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

Graph FileHandler::generate_complete_uniform_random_graph(int n,
                                                       double lambda,
                                                       bool use_seed,
                                                  uint64_t seed)
{
    // using namespace std;

    if (n <= 0) {
        cerr << "Error: number of nodes must be > 0\n";
        return Graph{};
    }

    mt19937_64 gen(use_seed ? seed : random_device{}());
    uniform_real_distribution<Cost> dist(lambda);

    Graph graph(n);

    // Create nodes 
    for (int i = 0; i < n; ++i) {
        Node node;
        node.id = i;
        node.latitude = 0.0;   
        node.longitude = 0.0;
        graph.set_node(node.id, node);
    }

    // Create bidirectional edges with random weights
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            Edge edge, reverse_edge;
            edge.src = i;
            edge.trg = j;
            reverse_edge.src = j;
            reverse_edge.trg = i;

            Cost w = dist(gen);
            edge.cost = w;
            reverse_edge.cost = w;

            graph.set_edge(edge, reverse_edge);
        }
    }

    return graph;
}

void FileHandler::write_to_metis_format(const Graph &graph, const string &file_path)
{
    vector<Node> nodes = graph.get_all_nodes();

    ofstream outputFile(file_path);
    if (!outputFile.is_open())
    {
        cerr << "Error opening file for writing: " << file_path << endl;
        return;
    }
    const int n = (int)graph.number_of_nodes();

    //adder header line
    outputFile << int(graph.number_of_nodes()) << " " << (int)graph.number_of_edges() / 2 << endl;

    //create a file in METIS format for processing Node Order
    for (int line = 1; line <= n; ++line)
    {
        // remap last line to old 0
        const int old_id = (line == n ? 0 : line); 
        const auto &adjacent_nodes = graph.neighbors(old_id);
        for (const auto &neighbor : adjacent_nodes)
        {
            int v_old = (int)neighbor.first;

            // remap neighbor 0 to n
            int v_new = (v_old == 0 ? n : v_old); 
            outputFile << v_new << ' ';
        }
        outputFile << '\n';
    }

    outputFile.close();
}

Ordering FileHandler::read_kahip_output(const string &file_path, int num_nodes)
{
    ifstream in(file_path);
    if (!in)
        throw runtime_error("cannot open " + file_path);

    if (!(in >> num_nodes))
        throw runtime_error("bad header");

    // store node order 
    Ordering ordering;

    // add nodes in order of contraction (low to high)
    ordering.rank_of_node.assign(num_nodes, 0);

    // Note down rank of node
    ordering.node_of_rank.assign(num_nodes, 0);

    for (int i = 0; i < num_nodes; ++i)
    {
        int rank, nodeId;
        if (!(in >> nodeId >> rank))
            throw runtime_error("bad line" + to_string(i + 2));

        if (nodeId == num_nodes)
            nodeId = 0; // check for 0th node 
        ordering.node_of_rank[rank - 1] = nodeId;
        ordering.rank_of_node[nodeId] = rank;
    }

    return ordering;
}
