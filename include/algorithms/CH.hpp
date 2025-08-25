#pragma once
#include <core/types.hpp>
#include <data_structures/graph.hpp>
#include <queue>
#include <utils/utils.hpp>
#include <algorithms/dijkstra.hpp>

class CH {
    private:
    // Implementation of Contraction Hierarchies
        Graph& graph;
        Utils utils; // utility functions for CH
        Dijkstra dijkstra; // for witness searches
        unordered_map<NodeId, vector<pair<Shortcut, ShortcutOpType>>> shortcutsCache;
        vector<int> currentEdgeDiffs;
        vector<NodeId> rank_order;
        int contractedNodes = 0;

        // At top or inside CH class

        // For deduping neighbors to recompute (epoch trick)
        std::vector<uint32_t> seen;   // size N, init to 0 in ctor
        uint32_t epoch = 1;
    public:
        CH(Graph& graph);
        
        void preprocess();
        void compute_rank_order();
        vector<int> get_rank_order();
        void recompute_edge_differences(vector<NodeId> nodes, NodePQ& pq);
        int compute_edge_difference(NodeId nodeId);
        int calculate_shortcuts(NodeId nodeId);
        void build_upward_adj();
        void build_downward_adj();
        vector<NodeId> query(NodeId src, NodeId dst);
};
