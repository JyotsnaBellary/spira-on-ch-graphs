#include <shortest-path/spira.hpp>
#include <algorithm>
#include <iostream>

// Implementation of Spira's single-source shortest path algorithm
Spira::Spira(CH_Graph &graph) : graph(graph), 
                             next_index(graph.number_of_nodes(), 0) {}

// Move pointer to point to the lowest costing edge of every node in the adjacency list
void Spira::reset() {
    next_index.assign(graph.number_of_nodes(), 0);
}

// Returns the next outgoing edge of a node 
EdgeId Spira::next(NodeId nodeId) {
    if (nodeId < 0 || nodeId >= graph.number_of_nodes()) return INVALID_EDGE;

    const vector<EdgeId>& adjacent_neighbor_edges = graph.get_out_neighbors(nodeId);

    if (next_index[nodeId] < (int)adjacent_neighbor_edges.size()) {
        // return current, then advance
        return adjacent_neighbor_edges[next_index[nodeId]++]; 
    }
    return INVALID_EDGE;
}

// Forward scan inserts the next outgoing edge into priority Queue P
void Spira::forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq) {
    EdgeId edgeId = next(nodeId);
    if (edgeId != INVALID_EDGE) {
        const auto& edge = graph.get_edge(edgeId); 
        // Push (dist(u) + c(u,v), edgeId) so the queue stores edges, not vertices.
        pq.emplace(cost[edge.src] + edge.cost, edgeId);
    }
}

// Function to query for shortest paths between a src and dest
// Optionally query for SPT by giving dest = -1
SsspResult Spira::compute_shortest_path(NodeId src, NodeId dst)
{
    int number_of_nodes = graph.number_of_nodes();
    int num_of_pops = 0;

    // 1. Initialize distances and bookkeeping arrays.
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
    if (src < 0 || src >= number_of_nodes || dst >= number_of_nodes || dst < -1)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    // priority queue storing (tentative distance, edge)
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
        // Extract the edge with the smallest resulting distance.
        auto [curr_cost, edgeId] = pq.top();
        pq.pop();
        num_of_pops += 1;
        const Edge& edge = graph.get_edge(edgeId);
        NodeId node = edge.src;

        // Skip stale queue entries that were superseded by a shorter edge.
        if (curr_cost !=  cost[node] + edge.cost)
            continue;

        //call forward(u)
        forward(node, cost, pq);

        // If v not in S i.e. not yet settled
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

// Builds path from src to dst. Optionally returns SPT if dst = -1
SsspResult Spira::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops)
{
    SsspResult result;
    // Record pops for analysis later
    result.number_of_pops = num_of_pops;
    result.number_of_pops = num_of_pops;
    result.avg_pops_per_node =
                static_cast<double>(num_of_pops) / static_cast<double>(graph.number_of_edges());
    
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
    if (dst >= (int)prev.size() || cost[dst] >= (long long)(numeric_limits<long long>::max() / 8))
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
