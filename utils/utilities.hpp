#pragma once
#include <graph.hpp>
#include <types.hpp>
#include <filesystem>

class Utils
{
public:
    Utils() = default;

    // Returns a list of valid independent nodes for contraction based on delta (Median value)
    vector<NodeId> independent_nodes(const Graph &graph, NodePQ &pq,
                                     int delta,
                                     const vector<int> &score);
    Dist sat_add(Dist a, Dist b);

    //
    // static bool is_local_min(NodeId u, const Graph &g, const vector<int> &s);

    // Function to run KaHIP 
    filesystem::path run_kahip(const char *out_name = "level1");

    //
    int compute_active_median(const Graph &g,
                              const vector<int> &score,
                              vector<int> &scratch);
};
