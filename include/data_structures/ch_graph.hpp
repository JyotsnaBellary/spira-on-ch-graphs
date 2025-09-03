#pragma once
#include <unordered_map>
#include <vector>
#include "data_structures/node.hpp"
#include "data_structures/edge.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

struct CH_DijkstraResult {
    vector<NodeId> path;
    vector<NodeId> ch_path;
    Dist total_cost;
    vector<EdgeId> edge_ids;
    vector<EdgeId> ch_edge_ids;
    int number_of_pops;
};

struct CH_Graph {
    vector<Node> nodes;                         // size n, nodes[i].id == i
    vector<Edge> edges;                         // flat edge storage                    // 1=active, 0=inactive
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_edges_adj;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_edges_adj;
    vector<int> rank;
    vector<bool> is_shortcut;
    int shortcuts=0;

    CH_Graph() = default;
    inline const vector<Edge> &get_all_edges() const { return edges; }
    
    //resize nodes, edges, active and adj
    CH_Graph(vector<Node> number_of_nodes, vector<Edge> number_of_edges, vector<int> rank_order) {
        nodes = number_of_nodes;
        edges = number_of_edges;
        upward_edges_adj.resize(nodes.size());
        rank = rank_order;
        is_shortcut.resize(edges.size());
        //build upward and downward adj list
        build_upward_adj_Lists(rank_order); 
    }

    inline bool is_up(NodeId u, NodeId v) { return rank[v] > rank[u]; }   // your convention

    int num_nodes() const { return static_cast<int>(nodes.size()); }
    inline const Edge& get_edge(EdgeId id) const { return edges.at(id); }
    inline const int &get_number_of_shortcuts() const { return shortcuts; }

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
                if (edge.shortcut) {
                shortcuts +=1;
                is_shortcut[edge.id] = true;
                }
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

     
    // Assumptions:
    // - get_edge(EdgeId) -> const Edge&
    // - Edge has: EdgeId id; bool shortcut; struct { EdgeId e_uv, e_vw; } sc;

    // inline void append_unpacked(EdgeId eid, std::vector<EdgeId>& out) {
    // const Edge& e = get_edge(eid);
    // if (!e.shortcut) {                 // base case: leaf/original edge
    //     out.push_back(eid);
    //     return;
    // }
    // // expand children in order: (u->m), then (m->v)
    // // (No extra pushes — the recursion returns fully expanded leaves.)
    // append_unpacked(e.sc.e_uv, out);
    // append_unpacked(e.sc.e_vw, out);
    // }

    inline void append_unpacked(EdgeId eid,
                     std::vector<EdgeId>& edge_out,
                     std::vector<NodeId>& node_out,
                     bool first = false) 
{
    const Edge& e = get_edge(eid);
    // Recursively unpack if it's a shortcut
    if (e.shortcut) {
        append_unpacked(e.sc.e_uv, edge_out, node_out, first);
        append_unpacked(e.sc.e_vw, edge_out, node_out, false);
    } else {
        edge_out.push_back(eid);
        if (first) node_out.push_back(e.src);
        node_out.push_back(e.trg);
    }
}

    CH_DijkstraResult unpack_shortcuts(CH_DijkstraResult ch_dijkstraResult) {
        vector<EdgeId> edge_out;
        vector<EdgeId> node_out;
        edge_out.reserve(ch_dijkstraResult.edge_ids.size() * 2);    // rough guess; grows as needed
        bool first = true;
        for (EdgeId eid : ch_dijkstraResult.ch_edge_ids) {
        append_unpacked(eid, edge_out, node_out, first);
            // ch_dijkstraResult.edge_ids = out;
            // ch_dijkstraResult.edge_ids = out;
            first = false; // only put the src once
        }

            ch_dijkstraResult.edge_ids = std::move(edge_out);
            ch_dijkstraResult.path  = std::move(node_out);
    //    Debug print: show all edges as "src - trg"
//     for (EdgeId eid : ch_dijkstraResult.ch_edge_ids){
//         const Edge& e = edges[eid];
//         std::cout << e.src << " - " << e.trg << std::endl;
//     }
// cout << "++++++++++" << endl;
//     // Debug print: show all edges as "src - trg"
//     for (EdgeId eid : ch_dijkstraResult.edge_ids){
//         const Edge& e = edges[eid];
//         std::cout << e.src << " - " << e.trg << std::endl;
//     }

//     for (NodeId eid : ch_dijkstraResult.path){
//         // const Edge& e = edges[eid];
//         std::cout << eid << " - ";
//     }

        return ch_dijkstraResult;
    }




}; 
