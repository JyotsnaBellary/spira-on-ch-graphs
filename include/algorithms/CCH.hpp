#pragma once
#include <core/types.hpp>
#include <data_structures/graph.hpp>

class CCH {
public:
    // but here. my graph should be edited 
    CCH(Graph& graph);
    void preprocess();
    vector<NodeId> compute_contraction_order();
    void compute_lower_triangles(const vector<NodeId>& contraction_order);

    int add_shortcuts(const vector<NodeId>& neighbors, NodeId middleNode);
    // void compute_nested_dissection();
    void compute_nested_dissection(vector<int> nodes, vector<NodeId>& reverse_contracted_nodes);
    void add_shortcut(NodeId u, NodeId v, Weight w);
    // void remove_shortcut(NodeId u, NodeId v);
    // vector<pair<Shortcut, ShortcutOpType>>& get_shortcuts(NodeId nodeId);

private:
    Graph& graph;
    vector<Edge> shortcutsCache;
    static inline uint64_t pair_key(NodeId u, NodeId v) {
    if (u > v) std::swap(u, v);
    return (uint64_t)u << 32 | (uint32_t)v;
}
    std::unordered_map<uint64_t, size_t> shortcutPos;          // (u,v) -> index in shortcutsCache

    // std::unordered_map<NodeId, std::unordered_map<NodeId, size_t>> shortcutPositionLookup;
    vector<vector<NodeId>> lower_triangle_nodes;
    vector<int> node_ranks;
};
