#include "spira.hpp"
#include <algorithm>
#include <iostream>

Spira::Spira(Graph &graph) : graph(graph), 
                             next_index(graph.number_of_nodes(), 0) {}
                            //  is_sorted(graph.number_of_nodes(), false) {}

void Spira::reset() {
    next_index.assign(graph.number_of_nodes(), 0);
}

EdgeId Spira::next(NodeId nodeId) {
    if (nodeId < 0 || nodeId >= graph.number_of_nodes()) return INVALID_EDGE;

    const vector<EdgeId>& adjacent_neighbor_edges = graph.get_out_neighbors(nodeId);

    if (next_index[nodeId] < (int)adjacent_neighbor_edges.size()) {
        return adjacent_neighbor_edges[next_index[nodeId]++]; // return current, then advance
    }
    return INVALID_EDGE;
}

void Spira::forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq) {
    EdgeId edgeId = next(nodeId);
    if (edgeId != INVALID_EDGE) {
        const auto& edge = graph.get_edge(edgeId); // (u,v)
        pq.emplace(cost[edge.src] + edge.cost, edgeId);
    }
}

SsspResult Spira::compute_shortest_path(NodeId src, NodeId dst)
{
    int number_of_nodes = graph.number_of_nodes();
    int num_of_pops = 0;

    // 1. Initialize distances and priority queue.
    vector<Cost> cost(number_of_nodes, INF_COST);
    vector<NodeId> prev(number_of_nodes, -1);
    vector<EdgeId> via_edge(number_of_nodes, -1);
    vector<bool> in_S(number_of_nodes, false);

    // Check for trivial case
    if (src == dst && dst > 0)
    {
        cost[src] = 0;
        return SsspResult{.path = {src}, .total_cost = 0, .edge_ids = {}, .number_of_pops = num_of_pops};
    }

    // Check for valid node IDs
    if (src < 0 || src >= number_of_nodes || dst >= number_of_nodes)
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
    forward(src, cost, pq);
    int settled = 1;

    // While the queue is not empty:
    while (pq.size() > 0 && settled < number_of_nodes)
    {
        // Extract the node with the smallest distance.
        auto [curr_cost, edgeId] = pq.top();
        pq.pop();
        num_of_pops += 1;
        const Edge& edge = graph.get_edge(edgeId);
        NodeId node = edge.src;

        // Is this check needed?
        if (curr_cost !=  cost[node] + edge.cost)
            continue;

        //call forward(u)
        forward(node, cost, pq);

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

            forward(trg, cost, pq);
        }
        
    }
    // If dst == -1, build full shortest path tree
    if (dst == -1) {
        return build_path(prev, cost, via_edge, dst, num_of_pops);
    }

    // No path is found.
    return SsspResult{{}, -1, {}, {}, {}, {}, num_of_pops, 0, 0};
}

SsspResult Spira::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops)
{
    SsspResult result;
    result.number_of_pops = num_of_pops;
    result.number_of_pops = num_of_pops;

    result.avg_pops_per_node =
                static_cast<double>(num_of_pops) / static_cast<double>(graph.number_of_edges());

    if (dst < 0 || dst >= (int)prev.size() || cost[dst] >= (long long)(numeric_limits<long long>::max() / 8))
    {
        result.total_cost = -1; // unreachable
        return result;
    }

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
