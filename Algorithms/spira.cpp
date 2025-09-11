#include "spira.hpp"
#include <algorithm>
#include <iostream>

Spira::Spira(Graph &graph) : graph(graph), 
                             next_index(graph.number_of_nodes(), 0), 
                             is_sorted(graph.number_of_nodes(), false) {}

void Spira::reset() {
    next_index.assign(graph.number_of_nodes(), 0);
}

EdgeId Spira::next(NodeId nodeId) {
    if (nodeId < 0 || nodeId >= graph.number_of_nodes()) return INVALID_EDGE;

    // lazy sort: only if we have not exposed any edge of nodeId yet
    if (!is_sorted[nodeId] && next_index[nodeId] == 0) {
        graph.sort_neighbors(nodeId);     // function provided below
        is_sorted[nodeId] = true;
    }

    const vector<EdgeId>& adjacent_neighbor_edges = graph.get_neighbors(nodeId);

    if (next_index[nodeId] < (int)adjacent_neighbor_edges.size()) {
        return adjacent_neighbor_edges[next_index[nodeId]++]; // return current, then advance
    }
    return INVALID_EDGE;
}

void Spira::forward_out(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq) {
    EdgeId edgeId = next(nodeId);
    if (edgeId != INVALID_EDGE) {
        const auto& edge = graph.edges[edgeId]; // (u,v)
        pq.emplace(cost[edge.src] + edge.cost, edgeId);
    }
}

DijkstraResult Spira::compute_shortest_path(NodeId src, NodeId dst)
{
    int number_of_nodes = graph.number_of_nodes();
    int num_of_pops = 0;

    // 1. Initialize distances and priority queue.
    vector<Cost> cost(number_of_nodes, INF_COST);
    vector<NodeId> prev(number_of_nodes, -1);
    vector<EdgeId> via_edge(number_of_nodes, -1);
    vector<bool> in_S(number_of_nodes, false);

    // Check for trivial case
    if (src == dst)
    {
        cost[src] = 0;
        return DijkstraResult{.path = {src}, .total_cost = 0, .edge_ids = {}, .number_of_pops = num_of_pops};
    }

    // Check for valid node IDs
    if (src < 0 || src >= number_of_nodes || dst < 0 || dst >= number_of_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    // priority queue
    priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>> pq;

    // Initialize the source node
    reset();
    cost[src] = 0;
    in_S[src] = true;
    forward_out(src, cost, pq);
    int settled = 1;

    // While the queue is not empty:
    while (pq.size() > 0 && settled < number_of_nodes)
    {
        // Extract the node with the smallest distance.
        auto [curr_cost, edgeId] = pq.top();
        pq.pop();
        num_of_pops += 1;
        Edge& edge = graph.edges[edgeId];
        NodeId node = edge.src;

        // Is this check needed?
        if (curr_cost !=  cost[node] + edge.cost)
            continue;

        // If we reached the destination, reconstruct the path.
        // if (node == dst)
        // {
        //     return build_path(prev, cost, via_edge, dst, num_of_pops);
        // };

        //call forward(u)
        forward_out(node, cost, pq);

        // if v ∉ S
        NodeId trg = edge.trg;
        if(!in_S[trg]) {
            cost[trg] = curr_cost;
            prev[trg] = node;
            via_edge[trg] = edgeId;
            in_S[trg] = true;
            ++settled;

            // Early exit when destination finalized
            if (trg == dst)
                return build_path(prev, cost, via_edge, dst, num_of_pops);

            forward_out(trg, cost, pq);
        }
        
    }

    // No path is found.
    return DijkstraResult{{}, -1, {}, num_of_pops};
}

DijkstraResult Spira::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops)
{
    DijkstraResult result;
    result.number_of_pops = num_of_pops;

    if (dst < 0 || dst >= (int)prev.size() || cost[dst] >= (long long)(numeric_limits<long long>::max() / 8))
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
