#include "dijkstra.hpp"
#include <queue>
#include <algorithm>
#include <iostream>

Dijkstra::Dijkstra(Graph &graph) : graph(graph) {}

// Function to query for shortest paths between a src and dest.
// Optionally query for SPT by giving dest = -1
SsspResult Dijkstra::compute_shortest_path(NodeId src, NodeId dst)
{
    int num_nodes = graph.number_of_nodes();
    int num_of_pops = 0;
    int redundant_pops = 0;

    // Track how many times each node is popped
    std::vector<int> pops_per_node(num_nodes, 0);

    // 1. Initialize distances and priority queue.
    vector<Cost> dist(num_nodes, INF_COST);
    vector<NodeId> prev(num_nodes, -1);
    vector<EdgeId> viaEdge(num_nodes, -1);

    // priority queue
    priority_queue<pair<Cost, NodeId>, vector<pair<Cost, NodeId>>, greater<pair<Cost, NodeId>>> pq;

    // Check for trivial case
    if (src == dst)
    {
        dist[src] = 0;
        return SsspResult{
            .path = {src},
            .total_cost = 0,
            .edge_ids = {},
            .redundant_pops = 0,
            .avg_pops_per_node = 0.0,
            .number_of_pops = num_of_pops
        };
    }

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst >= num_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    // Initialize the source node
    dist[src] = 0;
    pq.push({0, src});

    // While the queue is not empty:
    while (pq.size() > 0)
    {
        // Extract the node with the smallest distance.
        auto [curr_dist, node] = pq.top();
        pq.pop();
        num_of_pops++;
        pops_per_node[node]++;

        // If we have already found a better path, skip this one.
        if (curr_dist != dist[node]){
            redundant_pops++;
            continue;
        }

        // If we reached the destination, reconstruct the path.
        if (node == dst && dst != -1)
        {
            return build_path(prev, dist, viaEdge, dst, num_of_pops, pops_per_node, redundant_pops);
        };

        // Explore neighbors
        for (EdgeId edgeId : graph.get_out_neighbors(node))
        {
            const Edge& edge = graph.get_edge(edgeId);
            NodeId adjacent_node = edge.trg;
            Cost new_dist = dist[node] + edge.cost;

            // new shortest distance found for an adjacent node
            if (new_dist < dist[adjacent_node])
            {
                dist[adjacent_node] = new_dist;
                prev[adjacent_node] = node;
                viaEdge[adjacent_node] = edge.id;
                pq.push({new_dist, adjacent_node});
            }
        }
    }

    // If dst == -1, build full shortest path tree
    if (dst == -1) {
        return build_path(prev, dist, viaEdge, dst, num_of_pops, pops_per_node, redundant_pops);
    }

    // No path is found.
    return SsspResult{{}, -1, {}, {}, {}, {}, redundant_pops,
                      static_cast<double>(num_of_pops) / num_nodes, num_of_pops};
}

// Builds path from src to dst. Optionally returns SPT if dst = -1
SsspResult Dijkstra::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops, const vector<int> &pops_per_node, int redundant_pops)
{
    SsspResult result;

    // Record pops for analysis later
    result.number_of_pops = num_of_pops;
    result.redundant_pops = redundant_pops;
    result.avg_pops_per_node =
                static_cast<double>(num_of_pops) / static_cast<double>(graph.number_of_nodes());

    if (dst < 0) {
        // For SPT, we don't fill single-path fields
        result.total_cost = 0;
        result.path = {};
        result.edge_ids = {};

        // Fill SPT fields
        result.parent = prev;
        result.distance = cost;
        result.via_edge = viaEdge;

        return result;
    }
    
    // Check if dst is unreachable
    if (dst >= (int)prev.size() || cost[dst] == INF_COST)
    {
        result.total_cost = -1; // unreachable
        return result;
    }
    
    // Reconstruct nodes (backwards)
    vector<int> rev_nodes;
    vector<EdgeId> rev_edges;

    for (int v = dst; v != -1; v = prev[v])
    {
        rev_nodes.push_back(v);
        if (prev[v] != -1 && viaEdge[v] != INVALID_EDGE)
        {
            // edge used to reach v
            rev_edges.push_back(viaEdge[v]); 
        }
    }

    reverse(rev_nodes.begin(), rev_nodes.end());
    reverse(rev_edges.begin(), rev_edges.end());

    result.path = move(rev_nodes);
    result.edge_ids = move(rev_edges);
    result.total_cost = cost[dst];

    return result;
}
