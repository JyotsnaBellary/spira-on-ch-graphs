#include "dijkstra.hpp"
#include <queue>
#include <algorithm>
#include <iostream>

Dijkstra::Dijkstra(Graph &graph) : graph(graph) {}


// what kind of function is this?
// Dijkstra::~Dijkstra()
// {
// }



DijkstraResult Dijkstra::compute_shortest_path(NodeId src, NodeId dst)
{
    int num_nodes = graph.number_of_nodes();
    int num_of_pops = 0;

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
        return DijkstraResult{.path = {src}, .total_cost = 0, .edge_ids = {}, .number_of_pops = num_of_pops};
    }

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes)
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
        num_of_pops += 1;

        // If we have already found a better path, skip this one.
        if (curr_dist != dist[node])
            continue;

        // If we reached the destination, reconstruct the path.
        if (node == dst)
        {
            return build_path(prev, dist, viaEdge, dst, num_of_pops);
        };

        // Explore neighbors
        for (EdgeId edgeId : graph.get_out_neighbors(node))
        {
            const Edge& edge = graph.get_edge(edgeId);
            NodeId adjacent_node = edge.trg;
            Cost new_dist = dist[node] + edge.cost;
            if (new_dist < dist[adjacent_node])
            {
                dist[adjacent_node] = new_dist;
                prev[adjacent_node] = node;
                viaEdge[adjacent_node] = edge.id;
                pq.push({new_dist, adjacent_node});
            }
        }
    }

    // No path is found.
    return DijkstraResult{{}, -1, {}, num_of_pops};
}

DijkstraResult Dijkstra::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops)
{
    DijkstraResult result;
    result.number_of_pops = num_of_pops;

    if (dst < 0 || dst >= (int)prev.size() || cost[dst] == INF_COST)
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
            rev_edges.push_back(viaEdge[v]); // edge used to reach v
        }
    }

    reverse(rev_nodes.begin(), rev_nodes.end());
    reverse(rev_edges.begin(), rev_edges.end());

    result.path = move(rev_nodes);
    result.edge_ids = move(rev_edges);
    result.total_cost = cost[dst]; // keep Dist in the struct

    return result;
}
