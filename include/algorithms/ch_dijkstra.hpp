#pragma once
#include <data_structures/ch_graph.hpp>
#include "dijkstra.hpp"



class CH_Dijkstra {
    public:
        CH_Dijkstra(const CH_Graph& graph);
        CH_DijkstraResult compute_shortest_path(NodeId src, NodeId dst);
        CH_DijkstraResult build_ch_path(const vector<int>& prev_forward, const vector<int>& prev_backward, const vector<EdgeId>& viaEdge_forward, const vector<EdgeId>& viaEdge_backward, NodeId best_Node, Dist best_dist, int number_of_pops);
        // vector<EdgeId> unpack_shortcut(<Edge> edges);
        // CH_DijkstraResult build_ch_path(const vector<int>& prev_forward, const vector<int>& prev_backward, const vector<EdgeId>& viaEdge_forward, const vector<EdgeId>& viaEdge_backward, NodeId best_node, Dist best_dist);
        private:
        const CH_Graph& graph;

        vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_adj;
        // vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_adj;

        void build_upward_adj();
        // void build_downward_adj();

};


    
