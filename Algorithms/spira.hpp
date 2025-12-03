#pragma once
#include <graph.hpp>
#include "sssp_result.hpp"
#include <queue>

class Spira
{
    private:
        Graph& graph;

        vector<int> next_index;

    public:
        Spira(Graph& graph);

        void forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);
        
        void reset();
        EdgeId next(NodeId nodeId);

        //Computes shortest path between src and dst
        SsspResult compute_shortest_path(NodeId src, NodeId dst);
        
        //builds result path
        SsspResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops);
};
