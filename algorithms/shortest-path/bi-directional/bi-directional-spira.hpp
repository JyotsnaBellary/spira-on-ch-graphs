#pragma once

#include <ch-graph.hpp>
#include <sssp-result.hpp>
#include <types.hpp>
#include <queue>

class Bi_Directional_Spira
{
    public:
        explicit Bi_Directional_Spira(CH_Graph& graph);

        Bi_Directional_Dijkstra_Result compute_shortest_path(NodeId src, NodeId dst);

        Bi_Directional_Dijkstra_Result build_ch_path(
            const vector<NodeId>& prev_forward,
            const vector<NodeId>& prev_backward,
            const vector<EdgeId>& via_edge_forward,
            const vector<EdgeId>& via_edge_backward,
            NodeId best_node,
            Dist best_dist,
            int number_of_pops);

    private:
        CH_Graph& graph;
        vector<int> next_index_forward;
        vector<int> next_index_backward;

        void reset();
        EdgeId next(NodeId nodeId, vector<int>& next_index);
        void forward(
            NodeId nodeId,
            const vector<Cost>& cost,
            vector<int>& next_index,
            EdgePQ& pq);

        bool should_expand_forward(
            const EdgePQ& pq_forward,
            const EdgePQ& pq_backward,
            bool stop_f,
            bool stop_b) const;
};
