#include "algorithms/dijkstra.hpp"
#include "core/types.hpp"
#include <climits>
#include <queue>
#include <algorithm>
#include <iostream>
#include <utils/utils.hpp>

Dijkstra::Dijkstra(const Graph graph): graph(graph), dist_(graph.num_nodes(), INF) { touched_.reserve(1024); }

pair<bool, ShortcutOpType> Dijkstra::shortcut_search(NodeId src, NodeId dst, NodeId bannedNode, Dist distanceCap) {
    int num_nodes = graph.num_nodes();

    //Safety Check
    if (src == bannedNode || dst == bannedNode) return {false, ShortcutOpType::SKIP};

    // Quick direct-edge witness (<= cap means no shortcut needed)
    bool   has_direct   = false;
    Dist   direct_cost  = INF;
    EdgeId direct_eid   = INVALID_EDGE; 
    const auto& m = graph.neighbors(src);          // <-- public API
    auto it = m.find(dst);
    if (it != m.end()) {
        direct_cost = static_cast<Dist>(it->second.second);
        // direct_eid = it->second.first;

        //but is this used anywhere?
        has_direct = true;
        if (direct_cost <= distanceCap) {
            return {false, ShortcutOpType::SKIP};
        }
    }

    //priority queue
    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq;

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes) {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE);   // exit(1) also fine
    }

    // --- reuse dist_ without O(n) clear ---
    if ((int)dist_.size() != num_nodes) {                
        dist_.assign(num_nodes, INF);
        touched_.clear();
    } else {

        // reset only what we touched last time
        for (NodeId v : touched_) dist_[v] = INF; 
        touched_.clear();
    }

    dist_[src] = 0;
    touched_.push_back(src);
    pq.push({0, src});

    // While the queue is not empty:
    while(!pq.empty()) {
        // Extract the node with the smallest distance.
        auto [curr_dist, node] = pq.top();
        pq.pop();

        // If we have already found a better path, skip this one.
        if (curr_dist != dist_[node]) continue;

         // If we reached the destination, reconstruct the path.
        if (node == dst) {
            if (curr_dist >= distanceCap){
                return {true, has_direct ? ShortcutOpType::REPLACE : ShortcutOpType::ADD};
            }
            return {false, ShortcutOpType::SKIP};
        };

        // If best known distance already >= cap, no witness path <= cap is possible
        if (curr_dist > distanceCap) {
            // Decide ADD vs REPLACE based on presence of a (heavier) direct edge.
            return {true, has_direct ? ShortcutOpType::REPLACE : ShortcutOpType::ADD};
        } 

        // Explore neighbors
        for (auto& [adjacent_node, edgeInfo] : graph.neighbors(node)) {
            if (adjacent_node == bannedNode) continue; // skip if banned
            if (graph.is_active(adjacent_node) == false) continue; // skip if inactive
            Dist new_dist = dist_[node] + static_cast<Dist>(edgeInfo.second);
            if (new_dist < dist_[adjacent_node] && new_dist <= distanceCap) {
                if (dist_[adjacent_node] == INF) touched_.push_back(adjacent_node);
                dist_[adjacent_node] = new_dist;
                pq.push({new_dist, adjacent_node});
            }
        }
    }

    // Decide ADD vs REPLACE based on the direct edge presence and its cost (> cap here).
    return {true, has_direct ? ShortcutOpType::REPLACE : ShortcutOpType::ADD};
}

DijkstraResult Dijkstra::compute_shortest_path(NodeId src, NodeId dst) {
    int num_nodes = graph.num_nodes();
    int num_of_pops = 0;
    // 1. Initialize distances and priority queue.
    vector<Dist> dist(num_nodes, INF);
    vector<int> prev(num_nodes, -1);
    vector<EdgeId> viaEdge(num_nodes, -1);

    //priority queue
    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq;

    // Check for trivial case
    if (src == dst) {
        dist[src] = 0;
        return DijkstraResult{.path = {src}, .total_cost = 0, .edge_ids = {}, .number_of_pops = num_of_pops};
    }

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes) {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE);   // exit(1) also fine
    }

    // Initialize the source node
    dist[src] = 0;
    pq.push({0, src});

    // While the queue is not empty:
    while(pq.size() > 0) {
        // Extract the node with the smallest distance.
        auto [curr_dist, node] = pq.top();
        pq.pop();
        num_of_pops += 1;

        // If we have already found a better path, skip this one.
        if (curr_dist != dist[node]) continue;

        // If we reached the destination, reconstruct the path.
        if (node == dst) {
            return build_path(prev, dist, viaEdge, dst, num_of_pops);
        };

        
        // Explore neighbors
        for (auto& [adjacent_node, edgeInfo] : graph.neighbors(node)) {
            Dist new_dist = dist[node] + static_cast<Dist>(edgeInfo.second);
            if (new_dist < dist[adjacent_node]) {
                dist[adjacent_node] = new_dist;
                prev[adjacent_node] = node;
                viaEdge[adjacent_node] = edgeInfo.first; 
                pq.push({new_dist, adjacent_node});
            }
        }
    }

    // No path is found.
    return DijkstraResult{ {}, -1, {}, num_of_pops};
}

DijkstraResult Dijkstra::build_path(const vector<int>& prev, const vector<Dist>& dist, const vector<EdgeId>& viaEdge, NodeId dst, int num_of_pops) 
{
    DijkstraResult result;
    result.number_of_pops = num_of_pops;

    if (dst < 0 || dst >= (int)prev.size() || dist[dst] >= (long long) (numeric_limits<long long>::max()/8)) {
        result.total_cost = -1; // unreachable
        return result;
    }

    // Reconstruct nodes (backwards)
    vector<int> rev_nodes;
    vector<EdgeId> rev_edges;

    for (int v = dst; v != -1; v = prev[v]) {
        rev_nodes.push_back(v);
        if (prev[v] != -1 && viaEdge[v] != INVALID_EDGE) {
            rev_edges.push_back(viaEdge[v]); // edge used to reach v
        }
    }

    reverse(rev_nodes.begin(), rev_nodes.end());
    reverse(rev_edges.begin(), rev_edges.end());

    result.path = move(rev_nodes);
    result.edge_ids = move(rev_edges);
    result.total_cost = dist[dst];  // or keep Dist in the struct

    return result;
}