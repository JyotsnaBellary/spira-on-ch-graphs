// #pragma once

// #include <vector>

#include "graph.hpp"

// constructors
Graph::Graph() = default;
Graph::Graph(int number_of_nodes)
        : nodes(number_of_nodes),
          edges(),
          out_adjacency_list(number_of_nodes),
          in_adjacency_list(number_of_nodes) {}

// Graph Properties
int Graph::number_of_nodes() const { 
    return (int)nodes.size(); 
}

int Graph::number_of_edges() const { 
    return (int)edges.size();
}

// Node Mutator
void Graph::set_node(const Node &node) { 
    nodes[node.id] = node; 
}

// Edge Mutator
void Graph::set_edge(Edge &edge, Edge &rev_edge)
{
    EdgeId eid = static_cast<EdgeId>(edges.size());
    edge.id = eid; // ensure edge has a valid id
    edges.emplace_back(edge);

    rev_edge.id = eid + 1; // ensure reverse edge has a valid id
    edges.emplace_back(rev_edge);

    add_edge_adj(edge);
    add_edge_adj(rev_edge);
}

// Getters
const Node& Graph::get_node(NodeId &nodeId) const {
    return nodes[nodeId];
}

const Edge& Graph::get_edge(EdgeId &edgeId) const {
    return edges[edgeId];
}

const vector<Node>& Graph::get_nodes() const {
    return nodes;
}

const vector<Edge>& Graph::get_edges() const {
    return edges;
}

bool Graph::valid_node(NodeId u) const
        {
            return u >= 0 && u < number_of_nodes();
        }


// Adds edge to out_adjacency_list
void Graph::add_edge_adj(Edge &edge)
{
        if (!valid_node(edge.src))
            throw out_of_range("add_edge_adj: invalid node id " + to_string(edge.src));
        if (edge.id < 0 || edge.id >= static_cast<EdgeId>(edges.size()))
            throw out_of_range("add_edge_adj: invalid edge id " + to_string(edge.id));
        out_adjacency_list[edge.src].push_back(edge.id);
        in_adjacency_list[edge.trg].push_back(edge.id);
}

// Query neighbors of node
const vector<EdgeId> &Graph::get_out_neighbors(NodeId nodeId) const {
    return out_adjacency_list[nodeId];
}

const vector<EdgeId> &Graph::get_in_neighbors(NodeId nodeId) const {
    return in_adjacency_list[nodeId];
}

// #include "edge.hpp"
// #include <iostream>
// #include <algorithm>

// struct Graph
// {
//     vector<Node> nodes;
//     vector<Edge> edges;
//     // vector<vector<EdgeId>> adjacency_list;
//     vector<vector<EdgeId>> out_adjacency_list;
//     vector<vector<EdgeId>> in_adjacency_list;

//     // Constructors
//     Graph();
//     explicit Graph(int number_of_nodes);
//         // : nodes(number_of_nodes),
//         //   edges(),
//         //   adjacency_list(number_of_nodes) {}

//     static inline void ensure_size(vector<vector<EdgeId>> &adj, NodeId u)
//     {
//         if (u >= static_cast<NodeId>(adj.size()))
//             adj.resize(static_cast<size_t>(u) + 1);
//     }

//     inline void set_node(const Node &node) { nodes[node.id] = node; }
//     // Set Edge, Mainly used while creating shortcuts
//     inline void set_edge(Edge &edge, Edge &rev_edge)
//     {
//         EdgeId eid = static_cast<EdgeId>(edges.size());
//         edge.id = eid; // ensure edge has a valid id
//         edges.emplace_back(edge);

//         rev_edge.id = eid + 1; // ensure reverse edge has a valid id
//         edges.emplace_back(rev_edge);

//         add_edge_adj(edge.src, edge.id);
//         add_edge_adj(rev_edge.src, rev_edge.id);
//     }

//     inline bool valid_node(NodeId u) const
//     {
//         return u >= 0 && u < number_of_nodes();
//     }

//     inline void add_edge_adj(NodeId u, EdgeId eid)
//     {
//         if (!valid_node(u))
//             throw out_of_range("add_edge_adj: invalid node id " + to_string(u));
//         if (eid < 0 || eid >= static_cast<EdgeId>(edges.size()))
//             throw out_of_range("add_edge_adj: invalid edge id " + to_string(eid));
//         adjacency_list[u].push_back(eid);
//     }

//     int number_of_nodes() const { return (int)nodes.size(); }
//     const vector<EdgeId> &get_neighbors(NodeId nodeId) const { return adjacency_list[nodeId]; }

void Graph::sort_neighbors(NodeId nodeId)
{
    if (nodeId < 0 || nodeId >= number_of_nodes())
        return;
    auto &neighbor_list = out_adjacency_list[nodeId];
    sort(neighbor_list.begin(), neighbor_list.end(),
            [&](EdgeId a, EdgeId b)
            {
                return edges[a].cost < edges[b].cost; // optional tiebreak
            });
}

void Graph::sort_all_neighbors()
{
    for (NodeId nodeId = 0; nodeId < number_of_nodes(); nodeId++) sort_neighbors(nodeId);
}

void Graph::print_adj_simple()
{
    for (NodeId u = 0; u < static_cast<NodeId>(nodes.size()); ++u)
    {
        cout << "[" << u << "] : [";
        const auto &A = out_adjacency_list[u];
        for (size_t i = 0; i < A.size(); ++i)
        {
            EdgeId eid = A[i];
            cout << edges[eid].trg;
            if (i + 1 < A.size())
                cout << " ";
        }
        cout << "]\n";
    }
}
// };