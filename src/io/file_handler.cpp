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

void FileHandler::write_to_metis_format(const Graph& graph, const string& file_path) {
    vector<Node> nodes= graph.get_all_nodes();

    ofstream outputFile(file_path);
    if (!outputFile.is_open()) {
        cerr << "Error opening file for writing: " << file_path << endl;
        return;
    }
    const int n = (int)graph.num_nodes();
    
    outputFile << int(graph.num_nodes()) << " " << (int)graph.num_edges() / 2 << endl;

    for (int line = 1; line <= n; ++line) {
        const int old_id = (line == n ? 0 : line);           // remap last line to old 0
        const auto& adjacent_nodes = graph.neighbors(old_id);
        for (const auto& neighbor : adjacent_nodes) {
            int v_old = (int)neighbor.first;
            int v_new = (v_old == 0 ? n : v_old);             // remap neighbor 0 to n
            outputFile << v_new << ' ';
        }
        outputFile << '\n';
    }

    outputFile.close();
}

// void FileHandler::write_to_metis_format(const vector<int>& lineId_to_nodeId, const unordered_map<int,int>& nodeId_to_lineId, const Graph& graph, const string& file_path) {
//     ofstream outputFile(file_path);
//     if (!outputFile.is_open()) {
//         cerr << "Error opening file for writing: " << file_path << endl;
//         return;
//     }

//     // Create adjacent node list for lines
//     vector<vector<int>> line_adjacent_Nodes(lineId_to_nodeId.size());
//     int edgeCount = 0;
//     int nodeCount = 0;

//     for (int lineId = 0; lineId < lineId_to_nodeId.size(); ++lineId) {
//         if (lineId_to_nodeId[lineId] == -1) continue; //skip as lineId != 0

//         int node = lineId_to_nodeId[lineId];
//         nodeCount++;
//         const auto& adjacent_nodes = graph.neighbors(node);
//         for (const auto& neighbor : adjacent_nodes) {
//             NodeId neighbor_nodeId = neighbor.first;
//             if(!graph.is_active(neighbor_nodeId)) continue;

//             //check this again
//             // also check if neighbor is in this set of nodes
//             auto it = nodeId_to_lineId.find(neighbor_nodeId);
//             if (it == nodeId_to_lineId.end()) continue; // not a line node in this set
//             if (it->second == 0) continue;
//             //why is this changing here
//             line_adjacent_Nodes[lineId].push_back(it->second);
//             edgeCount++;
//         }
//     }

//     cout << "edgeCount: " << edgeCount << endl;
//     // Write the number of nodes and edges
//     outputFile << int(nodeCount) << " " << (int)edgeCount / 2 << endl;

//     // in the order of lines, like 1 two three, start adding the adjacent nodes to line i
//     for (size_t lineId = 1; lineId < line_adjacent_Nodes.size(); ++lineId) {
//     // for (const auto& lineId : lineId_to_nodeId) {
        
//         vector<int> neighbors = line_adjacent_Nodes[lineId];
//         for (const auto& neighbor : neighbors) {
//             outputFile << neighbor << " ";
//         }
//         outputFile << endl;
//     }

//     outputFile.close();
// }

// Kahip_Output FileHandler::read_kahip_output(const string& file_path, int num_nodes) {
//     Kahip_Output output;
//     ifstream inputFile(file_path);
//     if (!inputFile.is_open()) {
//         cerr << "Error opening file: " << file_path << endl;
//         return output;
//     }

//     string line;
//     for (int i = 1; i < num_nodes + 1; ++i) {
//         if (!getline(inputFile, line)) {
//             cerr << "Error reading line " << i + 1 << " from file: " << file_path << endl;
//             return output;
//         }

//         int part = stoi(line);
//         if (part == 0) {
//             output.left.push_back(i);
//         } else if (part == 1) {
//             output.right.push_back(i);
//         } else if (part == 2) {
//             output.separator.push_back(i);
//         } else {
//             cerr << "Unexpected partition value " << part << " on line " << i + 1 << endl;
//         }
//     }

//     inputFile.close();
//     return output;
// }

Ordering FileHandler::read_kahip_output(const string& file_path, int num_nodes) {
    ifstream in(file_path);
    if (!in) throw std::runtime_error("cannot open " + file_path);

    // int n; 
    if (!(in >> num_nodes)) throw std::runtime_error("bad header");
    Ordering ordering;
    // ordering.node_of_rank.assign(num_nodes, 0);
    ordering.rank_of_node.assign(num_nodes, 0);
    ordering.node_of_rank.assign(num_nodes, 0);

    for (int i = 0; i < num_nodes; ++i) {
        int rank, nodeId;
        if (!(in >> nodeId >> rank)) throw std::runtime_error("bad line" + std::to_string(i+2));

        if (nodeId == num_nodes) nodeId = 0;
        ordering.node_of_rank[rank-1] = nodeId;
        ordering.rank_of_node[nodeId] = rank;
    }

    // for (int i = 0; i < num_nodes; ++i) {
    //     int rank, nodeId;
    //     if (!(in >> rank >> nodeId)) throw std::runtime_error("bad line" + std::to_string(i+2));

    //     if (nodeId == num_nodes) nodeId = 0;
    //     ordering.node_of_rank.push_back(nodeId);
    //     ordering.rank_of_node[nodeId] = rank;
    // }
    return ordering;
}


// void FileHandler::export_to_CSV(Graph& graph, const string& nodesCSV, const string& edgesCSV, const vector<EdgeId>& shortest_path_edges) {
//     ofstream nodesFile(nodesCSV);
//     ofstream edgesFile(edgesCSV);

//     // Write nodes and edges to their respective files
//     nodesFile.close();
//     edgesFile.close();
// }
