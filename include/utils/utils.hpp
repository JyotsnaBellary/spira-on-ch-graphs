#pragma once
#include "data_structures/graph.hpp"
#include "core/types.hpp"
#include <filesystem>

class Utils {
    public:
        Utils() = default;
        std::vector<NodeId> independent_nodes(const Graph& graph, NodePQ& pq,
                                                   int delta,
                                                   const std::vector<int>& score);
        Dist sat_add(Dist a, Dist b);
        static bool is_local_min(NodeId u, const Graph& g, const vector<int>& s);

        filesystem::path run_kahip(const char* out_name = "level1");
        int compute_active_median(const Graph& g,
                          const std::vector<int>& score,
                          std::vector<int>& scratch);


};


// class GraphUtils {
//     public:
// };

