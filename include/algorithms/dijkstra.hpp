#pragma once
#include "data_structures/graph.hpp"

struct DijkstraResult {
    vector<NodeId> path;
    Dist total_cost;
    vector<EdgeId> edge_ids;
    int number_of_pops;
};

//Dijkstra Class for shortest path computation
class Dijkstra {
    Graph& graph;

    public:
        explicit Dijkstra(Graph& graph);
        //Computes shortest path between src and dst
        DijkstraResult compute_shortest_path(NodeId src, NodeId dst);
        //builds result path
        static DijkstraResult build_path(const vector<int>& prev, const vector<Dist>& dist, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops);

};









