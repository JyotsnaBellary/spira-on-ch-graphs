#pragma once
#include <unordered_map>
#include <vector>
#include "data_structures/node.hpp"
#include "data_structures/edge.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

struct CH_Graph {
    vector<Node> nodes;                         // size n, nodes[i].id == i
    vector<Edge> edges;                         // flat edge storage                    // 1=active, 0=inactive
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_edges_adj;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_edges_adj;
    vector<int> rank;

    CH_Graph() = default;
    inline const vector<Edge> &get_all_edges() const { return edges; }
    
    //resize nodes, edges, active and adj
    CH_Graph(vector<Node> number_of_nodes, vector<Edge> number_of_edges, vector<int> rank_order) {
        nodes = number_of_nodes;
        edges = number_of_edges;
        upward_edges_adj.resize(nodes.size());
        rank = rank_order;

        //build upward and downward adj list
        build_upward_adj_Lists(rank_order); 
    }

    inline bool is_up(NodeId u, NodeId v) { return rank[v] > rank[u]; }   // your convention

    int num_nodes() const { return static_cast<int>(nodes.size()); }
    inline const Edge &get_edge(EdgeId id) const { return edges[id]; }

    const unordered_map<NodeId, pair<EdgeId, Weight>>& up_neighbors(NodeId node) const {
        if (node < 0 || node >= static_cast<int>(upward_edges_adj.size())) {
            throw out_of_range("Invalid node ID");
        }
        return upward_edges_adj[node];
    }

    void build_upward_adj_Lists(vector<int> rank_order) {
        for (const auto& edge : edges) {
            if (is_up(edge.src, edge.trg)) {
                upward_edges_adj[edge.src][edge.trg] = {edge.id, edge.cost};
                // cout << "Added ";
                // if (edge.shortcut) cout << "shortcut ";
                // cout << "upward edge from " << edge.src << " to " << edge.trg << " (ID: " << edge.id << ", Cost: " << edge.cost << ")\n";
            } 
        }
    }

    void print_upward_adj() {
        for (NodeId src = 0; src < upward_edges_adj.size(); src++) {
            for (const auto& [trg, edgeInfo] : upward_edges_adj[src]) {
                cout << "Upward Edge from " << src << " to " << trg << " (ID: " << edgeInfo.first << ", Cost: " << edgeInfo.second << ")\n";
            }
        }
        
    }
}; 
