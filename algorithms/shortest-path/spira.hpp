#pragma once
#include <graph.hpp>
#include <ch-graph.hpp>
#include <sssp-result.hpp>
#include <queue>

class Spira
{
    private:
        CH_Graph& graph;

        // Pointer to track which outgoing edge should be explored next.
        vector<int> next_index;

    public:
        Spira(CH_Graph& graph);

        // Inserts the next outgoing edge into priority queue P
        void forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);
        
        // Move pointer to point to the lowest costing edge of every node in the adjacency list
        void reset();

        // Returns next outgoing edge of a node
        EdgeId next(NodeId nodeId);

        //Computes shortest path between src and dst
        SsspResult compute_shortest_path(NodeId src, NodeId dst);
        SsspResult compute_shortest_path(NodeId src, NodeId dst, int threshold);
        
        //Builds result path
        SsspResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops);
};
