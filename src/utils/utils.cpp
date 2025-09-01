#include "utils/utils.hpp"

#include <climits>

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

// std::vector<NodeId> Utils::independent_nodes(const Graph& graph, NodePQ& pq_in, int delta,
//                                    const std::vector<int>& score) {

//     // NodePQ q = pq_in;                    // preview: don’t disturb the real PQ
//     std::vector<NodeId> batch;
//     std::vector<char>   blocked(graph.num_nodes(), 0);

//     // find current min (skip stale/ inactive)
//     int m = INT_MAX;
//     while (!pq_in.empty()) {
//         auto [ed, u] = pq_in.top(); pq_in.pop();
//         if (!graph.is_active(u)) continue;
//         if (ed != score[u])     continue;   // lazy stale skip
//         m = ed; break;
//     }
//     if (m == INT_MAX) return batch;

//     const int T = m + delta;               // threshold

//     // Now scan the rest
//     // q = pq_in;                             // restart scan from top
//     while (!pq_in.empty()) {
//         auto [ed, u] = pq_in.top(); pq_in.pop();
//         if (!graph.is_active(u)) continue;
//         if (ed != score[u])     continue;  // stale
//         if (ed > T)             continue;  // above slack threshold
//         if (blocked[u])         continue;
//         if (!is_local_min(u, graph, score)) continue; // optional

//         // accept u into MIS
//         batch.push_back(u);
//         for (const auto& [v, e] : graph.neighbors(u)) {
//             if (graph.is_active(v)) blocked[v] = 1;
//         }
//     }
//     return batch;
// }

int Utils::compute_active_median(const Graph& g,
                          const std::vector<int>& score,
                          std::vector<int>& scratch) {
    scratch.clear();
    if (scratch.capacity() < (size_t)g.num_nodes())
        scratch.reserve(g.num_nodes());
    for (int u = 0; u < g.num_nodes(); ++u) {
        if (!g.is_active(u)) continue;
        int s = score[u];
        if (s == INT_MAX) continue;    // ignore unknown/stale
        scratch.push_back(s);
    }
    if (scratch.empty()) return INT_MAX;
    size_t k = scratch.size()/2;
    std::nth_element(scratch.begin(), scratch.begin()+k, scratch.end());
    return scratch[k];
}


// return vector of boolean with independent nodes marked as true or 1
std::vector<NodeId>
Utils::independent_nodes(const Graph& graph, NodePQ& pq_in, int T_abs,
                         const std::vector<int>& score)
{
    std::vector<NodeId> batch;
    std::vector<std::pair<int, NodeId>> stash; // window to process
    stash.reserve(1024);

    // --- 1) find current min m (skip stale/inactive), keep it in the window
    int m = INT_MAX;
    while (!pq_in.empty()) {
        auto [ed, u] = pq_in.top();   // POP here
        if (ed > T_abs) break;    
         pq_in.pop();                   // discard stale
        if (!graph.is_active(u) || ed != score[u]) continue; // discard stale
        m = ed;
        stash.push_back({ed, u});
        // break;
    }
    if (m == INT_MAX) return batch; // nothing usable

    // --- 2) pull a threshold window (keys <= T)
    // const int T = m + delta;
    // while (!pq_in.empty()) {
    //     if (pq_in.top().first > T) break;            // leave larger keys in the PQ
    //     auto [ed, u] = pq_in.top(); pq_in.pop();     // POP here
    //     if (!graph.is_active(u) || ed != score[u]) continue; // discard stale
    //     stash.push_back({ed, u});
    // }

    // --- 3) build MIS inside the window
    std::vector<char> blocked(graph.num_nodes(), 0); // only declare ONCE
    std::vector<char> chosen(graph.num_nodes(), 0);
    batch.reserve(stash.size());
    for (auto [ed, u] : stash) {
        if (chosen[u] || blocked[u]) continue;
        // Optional (often remove for speed):
        // if (!is_local_min(u, graph, score)) continue;

        batch.push_back(u);
        chosen[u] = 1;
        blocked[u] = 1;
        for (auto&& [v, e] : graph.neighbors(u))
            if (graph.is_active(v)) blocked[v] = 1;
    }

    // --- 4) reinsert non-selected window entries
    // std::vector<char> chosen(graph.num_nodes(), 0);
    // for (NodeId u : batch) chosen[u] = 1;
    for (auto [ed, u] : stash)
        if (!chosen[u]) pq_in.push({ed, u});

    return batch;
}



Dist Utils::sat_add(Dist a, Dist b)
{
    if (a >= INF || b >= INF)
        return INF;
    if (a > INF - b)
        return INF;
    return a + b;
}

// Local-min test (optional but recommended)
bool Utils::is_local_min(NodeId u, const Graph &g, const std::vector<int> &s)
{
    const int su = s[u];
    for (const auto &[v, e] : g.neighbors(u))
    {
        if (!g.is_active(v))
            continue;
        const int sv = s[v];
        if (sv < su || (sv == su && v < u))
            return false;
    }
    return true;
}

filesystem::path Utils::run_kahip(const char *out_name)
{
    // Ensure the Kahip executable is available
    const std::string kahip_executable = "kaffpa"; // or "kahip" based on your installation
    if (system((std::string("which ") + kahip_executable + " > /dev/null 2>&1").c_str()) != 0)
    {
        throw std::runtime_error("Kahip executable not found. Please ensure Kahip is installed and in your PATH.");
    }

    // Construct the command to run Kahip
    std::string command = kahip_executable;
    command += " --preconfiguration=fast";
    command += " --output_filename=" + std::string(out_name);
    command += " ./RoadNetworks/test.graph"; // Input graph file

    // Execute the command
    int ret = system(command.c_str());
    if (ret != 0)
    {
        throw std::runtime_error("Error running Kahip. Command: " + command);
    }

    return std::filesystem::path(out_name);
}
