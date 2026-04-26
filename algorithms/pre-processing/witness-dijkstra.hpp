#pragma once
#include <graph.hpp>
#include <shortest-path/dijkstra.hpp>

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









