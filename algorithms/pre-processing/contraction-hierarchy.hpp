#pragma once

#include <types.hpp>
#include <graph.hpp>
#include <queue>
#include <utilities.hpp>
#include <shortest-path/dijkstra.hpp>
#include <pre-processing/witness-dijkstra.hpp>

// Implementation of Contraction Hierarchies
class CH {
    private:
        Graph& graph;

        // utility functions for CH
        Utils utils; 

        // witness searches to decide if shortcuts are needed
        WitnessDijkstra witnessDijkstra; 

        // shortcut cache to keep track
        unordered_map<NodeId, vector<pair<Shortcut, ShortcutOpType>>> shortcuts_cache;
        vector<int> currentEdgeDiffs; //Keep track of edge differences in one contraction batch
        vector<NodeId> rank_order;
        int contractedNodes = 0;

        // For deduping neighbors to recompute
        vector<uint32_t> seen;   // size N, init to 0 in ctor
        uint32_t epoch = 1;

    public:
        CH(Graph& graph);
        
        //Functions W.R.T. to Contraction Order Computation
        int preprocess();
        int compute_rank_order();
        int compute_edge_difference(NodeId nodeId);
        void recompute_edge_differences(vector<NodeId> nodes, NodePQ& pq);
        int calculate_shortcuts(NodeId nodeId);

        //Helper functions
        vector<int> get_rank_order();
        void build_upward_adj();
        void build_downward_adj();

        // vector<NodeId> query(NodeId src, NodeId dst);
};
