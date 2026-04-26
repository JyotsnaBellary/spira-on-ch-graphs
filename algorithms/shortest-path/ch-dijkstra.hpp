#pragma once
#include <ch-graph.hpp>
#include <shortest-path/dijkstra.hpp>

class CH_Dijkstra
{
    public:
        //Initialised with Upwards Graph
        CH_Dijkstra(const CH_Graph &graph);

        //computes shortest path between src and dst using bidirectional Dijkstra
        CH_DijkstraResult compute_shortest_path(NodeId src, NodeId dst);

        //Unwinds the forward and backward edges into a directed set of edges ready for unpacking shortcuts
        CH_DijkstraResult build_ch_path(const vector<int> &prev_forward, const vector<int> &prev_backward, const vector<EdgeId> &viaEdge_forward, const vector<EdgeId> &viaEdge_backward, NodeId best_Node, Dist best_dist, int number_of_pops);

    private:
        const CH_Graph &graph;
        vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_adj;
};
