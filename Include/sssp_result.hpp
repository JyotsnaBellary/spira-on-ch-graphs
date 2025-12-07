#pragma once

#include <vector>
#include "types.hpp"

struct SsspResult {
    // --- Single path mode ---
    vector<NodeId> path;
    Cost total_cost;
    vector<EdgeId> edge_ids;

    // --- SPT mode (dst == -1) ---
    vector<NodeId> parent;       // parent[u]: predecessor in SPT
    vector<Cost> distance;       // distance[u]: cost from src to u
    vector<EdgeId> via_edge;     //via_edge[u]; u reached via edgeId

    // --- Performance & analysis metrics ---
    int redundant_pops = 0;            // number of redundant pops (curr_dist != dist[node])
    double avg_pops_per_node = 0.0;    // average number of pops per reached node
    int number_of_pops = 0;
    int number_of_Q_pops = 0;

    Cost median = 0.0;
    vector<bool> out_pertinent_edges;
    vector<bool> in_pertinent_edges;
    vector<bool> in_pertinent_edges_extracted_in_forward_phase;

    double runtime_ms = 0.0; 
};
