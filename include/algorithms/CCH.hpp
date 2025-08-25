#pragma once
#include <core/types.hpp>
#include <data_structures/graph.hpp>

class CCH {
public:
    // but here. my graph should be edited 
    CCH(Graph& graph);
    void preprocess();
    void compute_rank_order();
    void compute_nested_dissection();
    void add_shortcut(NodeId u, NodeId v, Weight w);
    void remove_shortcut(NodeId u, NodeId v);
    const vector<pair<Shortcut, ShortcutOpType>>& get_shortcuts(NodeId nodeId) const;

private:
    Graph& graph;
    vector<pair<Shortcut, ShortcutOpType>> shortcutsCache;
};
