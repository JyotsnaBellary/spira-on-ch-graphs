#pragma once
#include <graph.hpp>

struct SsspResult {
    // --- Single path mode ---
    vector<NodeId> path;
    Cost total_cost;
    vector<EdgeId> edge_ids;

    // --- SPT mode (dst == -1) ---
    vector<NodeId> parent;       // parent[u]: predecessor in SPT
    vector<Cost> distance;       // distance[u]: cost from src to u
    vector<EdgeId> via_edge;     //via_edge[u];

    // --- Performance & analysis metrics ---
    int redundant_pops = 0;            // number of redundant pops (curr_dist != dist[node])
    double avg_pops_per_node = 0.0;    // average number of pops per reached node
    int number_of_pops = 0;

};

class Dijkstra
{
    private:
        Graph& graph;

    public:
        Dijkstra(Graph& graph);

        //Computes shortest path between src and dst
        SsspResult compute_shortest_path(NodeId src, NodeId dst);

        //builds result path
        SsspResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops, const vector<int> &pops_per_node, int redundant_pops);
};

