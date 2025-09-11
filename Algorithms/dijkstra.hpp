#pragma once
#include <graph.hpp>

struct DijkstraResult {
    vector<NodeId> path;
    Cost total_cost;
    vector<EdgeId> edge_ids;
    int number_of_pops;
};

class Dijkstra
{
    private:
        Graph& graph;

    public:
        Dijkstra(Graph& graph);
        // ~Dijkstra();

        //Computes shortest path between src and dst
        DijkstraResult compute_shortest_path(NodeId src, NodeId dst);
        //builds result path
        DijkstraResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops);
};

