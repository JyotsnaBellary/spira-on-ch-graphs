#include "utils/utils.hpp"

#include <climits>

//return vector of boolean with independent nodes marked as true or 1
std::vector<NodeId> Utils::independent_nodes(const Graph& graph, const NodePQ& pq_in, int delta,
                                   const std::vector<int>& score) {
    // const int n = graph.num_nodes();
    // vector<char> active(n, 0), banned(n, 0);
    // for (int u = 0; u < n; ++u) active[u] = graph.is_active(u);

    // vector<bool> pick(n, false);
    // for (int u = 0; u < n; ++u) {
    //     if (!active[u] || banned[u]) continue;
    //     // pick u
    //     pick[u] = true;
    //     banned[u] = 1;
    //     // ban its neighbors (since you store both directions, out-neighbors suffice)
    //     for (const auto& [v, _] : graph.neighbors(u)) {
    //         if (active[v]) banned[v] = 1;
    //     }
    // }

    // return pick; // maximal independent set

    NodePQ q = pq_in;                    // preview: don’t disturb the real PQ
    std::vector<NodeId> batch;
    std::vector<char>   blocked(graph.num_nodes(), 0);

    // find current min (skip stale/ inactive)
    int m = INT_MAX;
    while (!q.empty()) {
        auto [ed, u] = q.top(); q.pop();
        if (!graph.is_active(u)) continue;
        if (ed != score[u])     continue;   // lazy stale skip
        m = ed; break;
    }
    if (m == INT_MAX) return batch;

    const int T = m + delta;               // threshold

    // Now scan the rest
    q = pq_in;                             // restart scan from top
    while (!q.empty()) {
        auto [ed, u] = q.top(); q.pop();
        if (!graph.is_active(u)) continue;
        if (ed != score[u])     continue;  // stale
        if (ed > T)             continue;  // above slack threshold
        if (blocked[u])         continue;
        if (!is_local_min(u, graph, score)) continue; // optional

        // accept u into MIS
        batch.push_back(u);
        for (const auto& [v, e] : graph.neighbors(u)) {
            if (graph.is_active(v)) blocked[v] = 1;
        }
    }
    return batch;
}

Dist Utils::sat_add(Dist a, Dist b) {
    if (a >= INF || b >= INF) return INF;
    if (a > INF - b) return INF;
    return a + b;
}

// Local-min test (optional but recommended)
bool Utils::is_local_min(NodeId u, const Graph& g, const std::vector<int>& s) {
    const int su = s[u];
    for (const auto& [v, e] : g.neighbors(u)) {
        if (!g.is_active(v)) continue;
        const int sv = s[v];
        if (sv < su || (sv == su && v < u)) return false;
    }
    return true;
}
