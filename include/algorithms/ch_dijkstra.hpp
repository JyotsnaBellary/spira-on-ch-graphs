#pragma once
#include <data_structures/ch_graph.hpp>
#include "dijkstra.hpp"

class CH_Dijkstra {
    public:
        CH_Dijkstra(const CH_Graph& graph);
        DijkstraResult compute_shortest_path(NodeId src, NodeId dst);
        DijkstraResult build_path(const vector<int>& prev, const vector<Dist>& dist, const vector<EdgeId>& viaEdge, NodeId dst);


    private:
        const CH_Graph& graph;

        vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_adj;
        vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_adj;

        void build_upward_adj();
        void build_downward_adj();
};
