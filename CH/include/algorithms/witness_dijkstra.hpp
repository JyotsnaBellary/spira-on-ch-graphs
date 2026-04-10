#pragma once
#include "data_structures/graph.hpp"
#include "dijkstra.hpp"

// the witness search needed for finiding where the shortcuts are needed. 

// struct DijkstraResult {
//     vector<NodeId> path;
//     Dist total_cost;
//     vector<EdgeId> edge_ids;
//     int number_of_pops;
// };

class WitnessDijkstra {
    const Graph& graph;
    vector<Dist> dist_;      // reused distances
    vector<NodeId> touched_; // nodes we set in this run

    public:
        explicit WitnessDijkstra(const Graph& graph);
        // DijkstraResult compute_shortest_path(NodeId src, NodeId dst);
        pair<bool, ShortcutOpType> shortcut_search(NodeId src, NodeId dst, NodeId bannedNode, Dist distanceCap);
        // DijkstraResult build_path(const vector<int>& prev, const vector<Dist>& dist, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops);

};









