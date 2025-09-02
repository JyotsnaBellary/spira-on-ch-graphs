#pragma once
#include <core/types.hpp>
#include <data_structures/graph.hpp>
#include <data_structures/ch_graph.hpp>

class CCH
{
public:
    // but here. my graph should be edited
    CCH(Graph &graph);
    CH_Graph preprocess();
    vector<NodeId> compute_contraction_order();
    void compute_lower_triangles(const vector<NodeId> &contraction_order);
    void set_shortcut_rank();
    void print_shortcuts_by_trg_order() const;
    int add_shortcuts(const vector<NodeId> &neighbors, NodeId middleNode);
    void compute_nested_dissection(vector<int> nodes, vector<NodeId> &reverse_contracted_nodes);
    void add_shortcut(NodeId u, NodeId v, Weight w);
    void customization(bool default_setting = true);
    void reset_customization_state_upward_only();

private:
    Graph &graph;
    vector<Edge> shortcutsCache;
    static inline uint64_t pair_key(NodeId u, NodeId v)
    {
        if (u > v)
            std::swap(u, v);
        return (uint64_t)u << 32 | (uint32_t)v;
    }
    std::unordered_map<uint64_t, size_t> shortcutPos; // (u,v) -> index in shortcutsCache

    vector<vector<ShortcutInfo>> lower_triangle_nodes;
    vector<int> rank_of_node;

    // rank -> index in shortcutsCache (global order by target, then middle ranks lexicographically)
    std::vector<size_t> shortcut_of_rank_by_trg;
    // inverse: index in shortcutsCache -> rank
    std::vector<int> rank_of_shortcut_by_trg;

    // per-shortcut cached, DESC-sorted middle-node ranks
    std::vector<std::vector<int>> mid_rank_sig_desc;
    // Optional safety in debug builds:
    inline void assert_upward(const Edge &e) const
    {
#ifndef NDEBUG
        assert(rank_of_node[e.src] < rank_of_node[e.trg]);
#endif
    }

    inline std::pair<NodeId, NodeId> oriented_uv(const Edge &e) const
    {
        // ensure v is "higher" by node_ranks
        if (rank_of_node[e.src] <= rank_of_node[e.trg])
            return {e.src, e.trg};
        return {e.trg, e.src};
    }

    inline NodeId effective_trg(const Edge &e) const
    {
        auto [u, v] = oriented_uv(e);
        return v;
    }

};
