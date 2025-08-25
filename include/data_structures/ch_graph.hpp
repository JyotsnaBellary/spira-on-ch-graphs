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

    CH_Graph() = default;
    
    //resize nodes, edges, active and adj
    CH_Graph(vector<Node> number_of_nodes, vector<Edge> number_of_edges, const vector<int>& rank_order) {
        nodes = number_of_nodes;
        edges = number_of_edges;
        // adj.resize(nodes.size());
        upward_edges_adj.resize(nodes.size());
        // downward_edges_adj.resize(nodes.size());

        //build upward and downward adj list
        build_upward_adj_Lists(rank_order);
    }

    int num_nodes() const { return static_cast<int>(nodes.size()); }

    const unordered_map<NodeId, pair<EdgeId, Weight>>& up_neighbors(NodeId node) const {
        if (node < 0 || node >= static_cast<int>(upward_edges_adj.size())) {
            throw out_of_range("Invalid node ID");
        }
        return upward_edges_adj[node];
    }

    // const unordered_map<NodeId, pair<EdgeId, Weight>>& down_neighbors(NodeId node) const {
    //     if (node < 0 || node >= static_cast<int>(downward_edges_adj.size())) {
    //         throw out_of_range("Invalid node ID");
    //     }
    //     return upward_edges_adj[node];
    // }

    // do I need this?
    // explicit CH_Graph(size_t n)
    //     : nodes(n), forward_adj(n), backward_adj(n) {
    //     for (NodeId i = 0; i < static_cast<NodeId>(n); ++i) {
    //         nodes[i].id = i;
    //         nodes[i].lat = nodes[i].lon = 0.0;
    //     }
    // }

    void build_upward_adj_Lists(vector<int> rank_order) {
        for (const auto& edge : edges) {
            if (rank_order[edge.src] < rank_order[edge.trg]) {
                upward_edges_adj[edge.src][edge.trg] = {edge.id, edge.cost};
                cout << "Added upward edge from " << edge.src << " to " << edge.trg << " (ID: " << edge.id << ", Cost: " << edge.cost << ")\n";
            } 
            // else {
            //     downward_edges_adj[edge.src][edge.trg] = {edge.id, edge.cost};
            //     cout << "Added downward edge from " << edge.src << " to " << edge.trg << " (ID: " << edge.id << ", Cost: " << edge.cost << ")\n";
            // }
        }
    }

    void print_upward_adj() {
        for (NodeId src = 0; src < upward_edges_adj.size(); src++) {
            for (const auto& [trg, edgeInfo] : upward_edges_adj[src]) {
                cout << "Upward Edge from " << src << " to " << trg << " (ID: " << edgeInfo.first << ", Cost: " << edgeInfo.second << ")\n";
            }
        }
        
    }
    // vector<Edge> upwardEdges;
    // vector<Edge> downwardEdges;
    
   

    //add getter and setters
//     inline const Node& get_node(NodeId id) const { return nodes[id]; }
//     inline void set_node(NodeId id, const Node& node) { nodes[id] = node; }
//     inline const vector<Node>& get_all_nodes() const { return nodes; }
//     inline const Edge& get_edge(EdgeId id) const { return edges[id]; }
//     inline const Edge& get_edge_by_src_dst(NodeId src, NodeId dst) const {
//         auto it = adj[src].find(dst);
//         if (it != adj[src].end()) {
//             return edges[it->second.first];
//         }
//         throw runtime_error("Edge not found");
//     }
//     inline void set_edge(Edge& edge) 
//     { 
//         EdgeId eid = static_cast<EdgeId>(edges.size());
//         edge.id = eid; // ensure edge has a valid id
//         edges.emplace_back(edge);
//         add_edge_adj(eid, edge.src, edge.trg, edge.cost);
//     }

//     inline int num_nodes() const { return static_cast<int>(nodes.size()); }
//     inline int num_edges() const { return static_cast<int>(edges.size()); }
//     // inline bool is_active(NodeId v) const { return active[v] != 0; }
//     // inline void deactivate(NodeId v) { active[v] = 0; }
//     // int num_active_neighbors(NodeId u) const { 
//     //     int count = 0;
//     //     for (const auto& [v, edgeInfo] : adj[u]) {
//     //         if (is_active(v)) count++;
//     //     }
//     //     return count;
//     // }
//     void build_adjacency_list(const vector<Edge>* customEdges = nullptr) {
//         const auto& es = customEdges ? *customEdges : edges;

//         adj.resize(nodes.size());
//         for (const auto& edge : es) {
//             adj[edge.src][edge.trg] = {edge.id, edge.cost};
//         }
//     }

//     //add edge to adjacency list
//     EdgeId add_edge_adj(EdgeId edge_id, NodeId u, NodeId v, Weight w) {
//         // EdgeId eid = edges.size();
//         // edges.emplace_back(eid, u, v, w, is_shortcut);
//         adj[u][v] = {edge_id, w};
//         return edge_id;
//     }

//     void update_edge(NodeId u, NodeId v, Weight w) {
//         auto it = adj[u].find(v);
//         if (it != adj[u].end()) {
//             EdgeId eid = it->second.first;
//             edges[eid].cost = w;
//             it->second.second = w; // update cost in adj
//         }

//         // edit_edge_adj(u, v, w);
//     }

//     // void print_adj_list() const {
//     //     for (NodeId u = 0; u < num_nodes(); ++u) {
//     //         if (is_active(u)) {
//     //             cout << "Node " << u << ": ";
//     //             for (const auto& [v, edgeInfo] : adj[u]) {
//     //                 cout << "(to " << v << ", cost " << edgeInfo.second << ") ";
//     //             }
//     //             cout << endl;
//     //         }
//     //     }
//     // }

//     const unordered_map<NodeId, pair<EdgeId, Weight>>& neighbors(NodeId u) const {
//         return adj[u];
//     }

//     int add_shortcuts(vector<pair<Shortcut, ShortcutOpType>>& shortcuts) {
//         // Add shortcuts for the given node
//         int count = 0;
//         for (const auto& [shortcut, shortcutOpType] : shortcuts) {
//             //create a seperate function for them.
//             if (shortcutOpType == ShortcutOpType::ADD) {
//                 Edge edge1;
//                 edge1.src = shortcut.u;
//                 edge1.trg = shortcut.w;
//                 edge1.cost = static_cast<Weight>(shortcut.cap);
//                 edge1.shortcut = true;
//                 edge1.sc = {
//                     //get the edgeIds. 
//                     edge1.sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
//                     edge1.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,
//                     edge1.sc.middle = shortcut.v,
//                 };
//                 set_edge(edge1);

//                 Edge edge2;
//                 edge2.src = shortcut.w;
//                 edge2.trg = shortcut.u;
//                 edge2.cost = static_cast<Weight>(shortcut.cap);
//                 edge2.shortcut = true;
//                 edge2.sc = {
//                     edge2.sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
//                     edge2.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,
//                     edge2.sc.middle = shortcut.v,
//                 };
//                 set_edge(edge2);

//                 count += 1;
//             } else if (shortcutOpType == ShortcutOpType::REPLACE) {
//                 // Find and update the existing edge. but uw and wu
//                 auto it = adj[shortcut.u].find(shortcut.w);
//                 if (it != adj[shortcut.u].end()) {
//                     EdgeId eid = it->second.first;
//                     edges[eid].cost = static_cast<Weight>(shortcut.cap);
//                     it->second.second = static_cast<Weight>(shortcut.cap); // update cost in adj
//                     edges[eid].shortcut = true;
//                     edges[eid].sc = {
//                         //get the edgeIds
//                         edges[eid].sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
//                         edges[eid].sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,
//                         edges[eid].sc.middle = shortcut.v,
//                     };
//                 }

//                 auto it1 = adj[shortcut.w].find(shortcut.u);
//                 if (it1 != adj[shortcut.w].end()) {
//                     EdgeId eid = it1->second.first;
//                     edges[eid].cost = static_cast<Weight>(shortcut.cap);
//                     it->second.second = static_cast<Weight>(shortcut.cap); // update cost in adj
//                     edges[eid].shortcut = true;
//                     edges[eid].sc = {
//                         //get the edgeIds
//                         edges[eid].sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
//                         edges[eid].sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,
//                         edges[eid].sc.middle = shortcut.v,
//                     };
//                 }
//             }
//         }

//         return count;
//     }

}; 
