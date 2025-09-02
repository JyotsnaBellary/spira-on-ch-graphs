#pragma once
#include <data_structures/ch_graph.hpp>
#include "dijkstra.hpp"

struct CH_DijkstraResult {
    vector<NodeId> path;
    vector<NodeId> ch_path;
    Dist total_cost;
    vector<EdgeId> edge_ids;
    vector<EdgeId> ch_edge_ids;
    int number_of_pops;
};

class CH_Dijkstra {
    public:
        CH_Dijkstra(const CH_Graph& graph);
        CH_DijkstraResult compute_shortest_path(NodeId src, NodeId dst);
        CH_DijkstraResult build_ch_path(const vector<int>& prev_forward, const vector<int>& prev_backward, const vector<EdgeId>& viaEdge_forward, const vector<EdgeId>& viaEdge_backward, NodeId best_Node, Dist best_dist, int number_of_pops);
        vector<EdgeId> unpack_shortcut(Edge edge);
        // CH_DijkstraResult build_ch_path(const vector<int>& prev_forward, const vector<int>& prev_backward, const vector<EdgeId>& viaEdge_forward, const vector<EdgeId>& viaEdge_backward, NodeId best_node, Dist best_dist);
        private:
        const CH_Graph& graph;

        vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_adj;
        // vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_adj;

        void build_upward_adj();
        // void build_downward_adj();

};
// Expand one directed CH edge-id e into original directed edges (left→right).

// inline void unpack_edge_iter(const std::vector<Edge>& edges,
//                              EdgeId e, std::vector<EdgeId>& out) {
//     std::vector<EdgeId> st; st.push_back(e);
//     cout << "called";
//     while (!st.empty()) {
//         EdgeId cur = st.back(); st.pop_back();
//         const Edge& ed = edges[cur];
//         if (!ed.shortcut) {
//             out.push_back(cur);                       // base edge, done
//         } else {
//             // IMPORTANT: children are for THIS direction (src->trg).
//             // Push second first, then first, so first is processed next (LIFO).
//             st.push_back(ed.sc.e_vw);                 // second half
//             st.push_back(ed.sc.e_uv);                 // first half
//         }
//     }
// }

// Expand the whole CH path (list of CH eids) to base eids
// inline std::vector<EdgeId>
// unpack_path_edges(const std::vector<Edge>& edges,
//                   const std::vector<EdgeId>& ch_eids) {
//     std::vector<EdgeId> orig;
//     orig.reserve(ch_eids.size() * 4);
//     for (EdgeId e : ch_eids) unpack_edge_iter(edges, e, orig);
//     return orig;
// }

// Base edges → node sequence
// inline std::vector<NodeId>
// edges_to_nodes(const std::vector<Edge>& edges, const std::vector<EdgeId>& eids) {
//     std::vector<NodeId> nodes;
//     if (eids.empty()) return nodes;
//     nodes.push_back(edges[eids.front()].src);
//     for (EdgeId e : eids) nodes.push_back(edges[e].trg);
//     return nodes;
// }

// Sum weight over base edges
// inline long long sum_weight(const std::vector<Edge>& edges,
//                             const std::vector<EdgeId>& eids) {
//     long long s = 0; for (EdgeId e : eids) s += edges[e].cost; return s;
// }


    
