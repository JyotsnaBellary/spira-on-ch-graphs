#include "io/file_handler.hpp"
#include <fstream>
#include <iostream>

Graph FileHandler::read_file(const string &filePath)
{
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
        inputFile >> node.id >> node.lat >> node.lon;
        graph.set_node(node.id, node);
    }

    // Read edges
    for (int i = 0; i < m; ++i)
    {
        Edge edge;
        Edge reverse_edge;

        inputFile >> edge.src >> edge.trg;
        edge.cost = 1;
        edge.shortcut = false;

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

void FileHandler::write_to_metis_format(const Graph &graph, const string &file_path)
{
    vector<Node> nodes = graph.get_all_nodes();

    ofstream outputFile(file_path);
    if (!outputFile.is_open())
    {
        cerr << "Error opening file for writing: " << file_path << endl;
        return;
    }
    const int n = (int)graph.num_nodes();

    //adder header line
    outputFile << int(graph.num_nodes()) << " " << (int)graph.num_edges() / 2 << endl;

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
