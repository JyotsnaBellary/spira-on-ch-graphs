#pragma once
#include "data_structures/graph.hpp"
#include "core/types.hpp"

class Utils {
    public:
        Utils() = default;
        std::vector<NodeId> independent_nodes(const Graph& graph, const NodePQ& pq,
                                                   int delta,
                                                   const std::vector<int>& score);
        Dist sat_add(Dist a, Dist b);
        static bool is_local_min(NodeId u, const Graph& g, const vector<int>& s);

};

// class GraphUtils {
//     public:
// };

