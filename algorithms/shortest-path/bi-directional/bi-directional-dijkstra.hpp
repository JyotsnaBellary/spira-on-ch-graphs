#pragma once
#include <ch-graph.hpp>
#include <shortest-path/dijkstra.hpp>
#include "sssp-result.hpp"
#include <enums.hpp>
#include <elimination-tree.hpp>
#include <node-scheduler.hpp>

class Bi_Directional_Dijkstra
{
    public:
        //Initialised with Upwards Graph
        Bi_Directional_Dijkstra(const CH_Graph &graph);

        // Bi_Directional_Dijkstra(const CH_Graph& graph,
        //                     const EliminationTree& elimination_tree);

                            bool take_forward(
    // SearchMode mode,
    NodePQ& pq_forward,
    NodePQ& pq_backward,
    bool stop_f,
    bool stop_b);

        //computes shortest path between src and dst using bidirectional Dijkstra
        Bi_Directional_Dijkstra_Result compute_shortest_path(NodeId src, NodeId dst);

        //Unwinds the forward and backward edges into a directed set of edges ready for unpacking shortcuts
        Bi_Directional_Dijkstra_Result build_ch_path(const vector<int> &prev_forward, const vector<int> &prev_backward, const vector<EdgeId> &viaEdge_forward, const vector<EdgeId> &viaEdge_backward, NodeId best_Node, Dist best_dist, int number_of_pops);

        

    private:
        const CH_Graph &graph;
        // vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_adj;
        
        const EliminationTree* elimination_tree = nullptr;
};
