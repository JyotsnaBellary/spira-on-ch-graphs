#pragma once

#include <vector>

#include "node.hpp"
#include "edge.hpp"
#include <iostream>
#include <algorithm>

struct Graph
{
    vector<Node> nodes;
    vector<Edge> edges;
    vector<vector<EdgeId>> adjacency_list;

    Graph() = default;

    // resize nodes, edges, active and adj
    // Graph(int number_of_nodes)
    // {
    //     nodes.resize(number_of_nodes);
    //     adjacency_list.resize(number_of_nodes);
    // }

    explicit Graph(int number_of_nodes)
        : nodes(number_of_nodes),
          edges(),
          adjacency_list(number_of_nodes) {}

    static inline void ensure_size(vector<vector<EdgeId>> &adj, NodeId u)
    {
        if (u >= static_cast<NodeId>(adj.size()))
            adj.resize(static_cast<size_t>(u) + 1);
    }

    inline void set_node(const Node &node) { nodes[node.id] = node; }
    // Set Edge, Mainly used while creating shortcuts
    inline void set_edge(Edge &edge, Edge &rev_edge)
    {
        EdgeId eid = static_cast<EdgeId>(edges.size());
        edge.id = eid; // ensure edge has a valid id
        edges.emplace_back(edge);

        rev_edge.id = eid + 1; // ensure reverse edge has a valid id
        edges.emplace_back(rev_edge);

        add_edge_adj(edge.src, edge.id);
        add_edge_adj(rev_edge.src, rev_edge.id);
    }

    inline bool valid_node(NodeId u) const
    {
        return u >= 0 && u < number_of_nodes();
    }

    inline void add_edge_adj(NodeId u, EdgeId eid)
    {
        if (!valid_node(u))
            throw out_of_range("add_edge_adj: invalid node id " + to_string(u));
        if (eid < 0 || eid >= static_cast<EdgeId>(edges.size()))
            throw out_of_range("add_edge_adj: invalid edge id " + to_string(eid));
        adjacency_list[u].push_back(eid);
    }

    int number_of_nodes() const { return (int)nodes.size(); }
    const vector<EdgeId> &get_neighbors(NodeId nodeId) const { return adjacency_list[nodeId]; }

    void sort_neighbors(NodeId nodeId)
    {
        if (nodeId < 0 || nodeId >= (int)adjacency_list.size())
            return;
        auto &neighbor_list = adjacency_list[nodeId];
        sort(neighbor_list.begin(), neighbor_list.end(),
             [&](EdgeId a, EdgeId b)
             {
                 return edges[a].cost < edges[b].cost; // optional tiebreak
             });
    }

    void print_adj_simple()
    {
        for (NodeId u = 0; u < static_cast<NodeId>(nodes.size()); ++u)
        {
            cout << "[" << u << "] : [";
            const auto &A = adjacency_list[u];
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
};