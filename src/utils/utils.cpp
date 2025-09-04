#include "utils/utils.hpp"

#include <climits>

int Utils::compute_active_median(const Graph& g,
                          const vector<int>& score,
                          vector<int>& scratch) {
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
    nth_element(scratch.begin(), scratch.begin()+k, scratch.end());
    return scratch[k];
}


// return vector of boolean with independent nodes marked as true or 1
vector<NodeId>
Utils::independent_nodes(const Graph& graph, NodePQ& pq_in, int T_abs,
                         const vector<int>& score)
{

    static vector<int> blocked_stamp, chosen_stamp;
    static int stamp = 1;

    const int n = graph.num_nodes();
    if ((int)blocked_stamp.size() < n) {
        blocked_stamp.resize(n, 0);
        chosen_stamp.resize(n, 0);
    }
    if (++stamp == INT_MAX) {                 // rare wraparound guard
        fill(blocked_stamp.begin(), blocked_stamp.end(), 0);
        fill(chosen_stamp.begin(),  chosen_stamp.end(), 0);
        stamp = 1;
    }

    auto is_blocked = [&](NodeId v){ return blocked_stamp[v] == stamp; };
    auto set_blocked = [&](NodeId v){ blocked_stamp[v] = stamp; };
    auto is_chosen  = [&](NodeId v){ return chosen_stamp[v] == stamp; };
    auto set_chosen = [&](NodeId v){ chosen_stamp[v] = stamp; };

    vector<NodeId> batch;
    vector<pair<int, NodeId>> stash; // window to process
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

    
    // --- 3) build MIS inside the window
    // vector<char> blocked(graph.num_nodes(), 0); // only declare ONCE
    // vector<char> chosen(graph.num_nodes(), 0);
    batch.reserve(stash.size());
    for (auto [ed, u] : stash) {
        if (is_chosen(u) || is_blocked(u)) continue;

        batch.push_back(u);
        // chosen[u] = 1;
        // blocked[u] = 1;
         set_chosen(u);
        set_blocked(u);
        for (auto&& [v, e] : graph.neighbors(u))
            if (graph.is_active(v)) set_blocked(v);
    }

    // --- 4) reinsert non-selected window entries
    // for (NodeId u : batch) chosen[u] = 1;
    for (auto [ed, u] : stash)
        if (!is_chosen(u)) pq_in.push({ed, u});

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
bool Utils::is_local_min(NodeId u, const Graph &g, const vector<int> &s)
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
    const string kahip_executable = "kaffpa"; // or "kahip" based on your installation
    if (system((string("which ") + kahip_executable + " > /dev/null 2>&1").c_str()) != 0)
    {
        throw runtime_error("Kahip executable not found. Please ensure Kahip is installed and in your PATH.");
    }

    // Construct the command to run Kahip
    string command = kahip_executable;
    command += " --preconfiguration=fast";
    command += " --output_filename=" + string(out_name);
    command += " ./RoadNetworks/test.graph"; // Input graph file

    // Execute the command
    int ret = system(command.c_str());
    if (ret != 0)
    {
        throw runtime_error("Error running Kahip. Command: " + command);
    }

    return filesystem::path(out_name);
}
