#pragma once

#include <vector>
#include "types.hpp"

struct SsspResult {
    // --- Single path mode ---
    std::vector<NodeId> path;
    Cost total_cost;
    std::vector<EdgeId> edge_ids;

    // --- SPT mode (dst == -1) ---
    std::vector<NodeId> parent;       // parent[u]: predecessor in SPT
    std::vector<Cost> distance;       // distance[u]: cost from src to u
    std::vector<EdgeId> via_edge;     //via_edge[u];

    // --- Performance & analysis metrics ---
    int redundant_pops = 0;            // number of redundant pops (curr_dist != dist[node])
    double avg_pops_per_node = 0.0;    // average number of pops per reached node
    int number_of_pops = 0;
    int number_of_Q_pops = 0;

    Cost median = 0.0;
    std::vector<bool> out_pertinent_edges;
    std::vector<bool> in_pertinent_edges;
    std::vector<bool> in_pertinent_edges_extracted_in_forward_phase;
};
