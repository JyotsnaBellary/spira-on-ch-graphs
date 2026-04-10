#pragma once
#include <core/types.hpp>
#include <data_structures/graph.hpp>
#include <data_structures/ch_graph.hpp>

struct CCH_Result
{
    int shortcuts;
    int avg_lower_triangles_per_edge;
    int maximum_triangles_edge;
};

class CCH
{
public:
    CCH(Graph &graph);
    CCH_Result preprocess();
    void compute_contraction_order();
    void compute_lower_triangles(const vector<NodeId> &contraction_order);
    void set_shortcut_rank();
    void print_shortcuts_by_trg_order() const;
    int add_shortcuts(const vector<NodeId> &neighbors, NodeId middleNode);
    void compute_nested_dissection(vector<int> nodes, vector<NodeId> &reverse_contracted_nodes);
    void add_shortcut(NodeId u, NodeId v, Weight w);
    void customization(bool default_setting = true, bool assign_random_weights = false);
    void reset_customization_update_weights(bool assign_random_weights = false, uint64_t seed = 0);
    const Graph &get_graph() { return graph; }
    const vector<int> get_ranks() { return rank_of_node; }

private:
    Graph &graph;
    int shortcuts = 0;
    vector<Edge> shortcutsCache;
    vector<int> contraction_order;
    static inline uint64_t pair_key(NodeId u, NodeId v)
    {
        if (u > v)
            swap(u, v);
        return (uint64_t)u << 32 | (uint32_t)v;
    }
    unordered_map<uint64_t, size_t> shortcutPos; // (u,v) -> index in shortcutsCache

    // stores {left edge, right edge, middleNode}
    vector<vector<ShortcutInfo>> lower_triangle_nodes;
    vector<int> rank_of_node;

    // rank -> index in shortcutsCache (global order by target, then order by src if same)
    vector<size_t> shortcut_of_rank_by_trg;

    // inverse: index in shortcutsCache -> rank
    vector<int> rank_of_shortcut_by_trg;

    // per-shortcut cached, DESC-sorted middle-node ranks
    vector<vector<int>> mid_rank_sig_desc;

    // Optional safety in debug builds:
    inline void assert_upward(const Edge &e) const
    {
#ifndef NDEBUG
        assert(rank_of_node[e.src] < rank_of_node[e.trg]);
#endif
    }
};
