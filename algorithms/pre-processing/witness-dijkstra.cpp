// #include "algorithms/dijkstra.hpp"
#include <types.hpp>
#include <climits>
#include <queue>
#include <algorithm>
#include <iostream>
#include <utilities.hpp>
#include <pre-processing/witness-dijkstra.hpp>

WitnessDijkstra::WitnessDijkstra(const Graph &graph) : graph(graph), dist_(graph.number_of_nodes(), INF) { touched_.reserve(1024); }

pair<bool, ShortcutOpType> WitnessDijkstra::shortcut_search(NodeId src, NodeId dst, NodeId bannedNode, Dist distanceCap)
{
    int num_nodes = graph.number_of_nodes();

    // Safety Check
    if (src == bannedNode || dst == bannedNode)
        return {false, ShortcutOpType::SKIP};

    // Quick direct-edge witness (<= cap means no shortcut needed)
    bool has_direct = false;
    Dist direct_cost = INF;
    EdgeId direct_eid = INVALID_EDGE;
    const auto &m = graph.neighbors(src); // <-- public API
    auto it = m.find(dst);
    if (it != m.end())
    {
        direct_cost = static_cast<Dist>(it->second.second);

        has_direct = true;
        if (direct_cost <= distanceCap)
        {
            return {false, ShortcutOpType::SKIP};
        }
    }

    // priority queue
    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq;

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    // --- reuse dist_ without O(n) clear ---
    if ((int)dist_.size() != num_nodes)
    {
        dist_.assign(num_nodes, INF);
        touched_.clear();
    }
    else
    {

        // reset only what we touched last time
        for (NodeId v : touched_)
            dist_[v] = INF;
        touched_.clear();
    }

    dist_[src] = 0;
    touched_.push_back(src);
    pq.push({0, src});

    // While the queue is not empty:
    while (!pq.empty())
    {
        // Extract the node with the smallest distance.
        auto [curr_dist, node] = pq.top();
        pq.pop();

        // If we have already found a better path, skip this one.
        if (curr_dist != dist_[node])
            continue;

        // If we reached the destination, reconstruct the path.
        if (node == dst)
        {
            if (curr_dist >= distanceCap)
            {
                return {true, has_direct ? ShortcutOpType::REPLACE : ShortcutOpType::ADD};
            }
            return {false, ShortcutOpType::SKIP};
        };

        // If best known distance already >= cap, no witness path <= cap is possible
        if (curr_dist > distanceCap)
        {
            // Decide ADD vs REPLACE based on presence of a (heavier) direct edge.
            return {true, has_direct ? ShortcutOpType::REPLACE : ShortcutOpType::ADD};
        }

        // Explore neighbors
        for (auto &[adjacent_node, edgeInfo] : graph.neighbors(node))
        {
            if (adjacent_node == bannedNode)
                continue; // skip if banned
            if (graph.is_active(adjacent_node) == false)
                continue; // skip if inactive
            Dist new_dist = dist_[node] + static_cast<Dist>(edgeInfo.second);
            if (new_dist < dist_[adjacent_node] && new_dist <= distanceCap)
            {
                if (dist_[adjacent_node] == INF)
                    touched_.push_back(adjacent_node);
                dist_[adjacent_node] = new_dist;
                pq.push({new_dist, adjacent_node});
            }
        }
    }

    // Decide ADD vs REPLACE based on the direct edge presence and its cost (> cap here).
    return {true, has_direct ? ShortcutOpType::REPLACE : ShortcutOpType::ADD};
}