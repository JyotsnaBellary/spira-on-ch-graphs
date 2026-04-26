#pragma once
#include <graph.hpp>
#include <ch-graph.hpp>
#include "sssp-result.hpp"


class Dijkstra
{
    private:
        CH_Graph& graph;

    public:
        Dijkstra(CH_Graph& graph);

        //Computes shortest path between src and dst
        SsspResult compute_shortest_path(NodeId src, NodeId dst);

        //builds result path
        SsspResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops, const vector<int> &pops_per_node, int redundant_pops);
};
