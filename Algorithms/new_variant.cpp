#include "new_variant.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

NewVariant::NewVariant(Graph &graph) : graph(graph), 
                             next_index_out_adj(graph.number_of_nodes(), 0),
                             next_index_in_adj(graph.number_of_nodes(), 0),
                             next_index_request(graph.number_of_nodes(),0),
                             in_pertinent_edges(graph.number_of_edges(), false),
                             out_pertinent_edges(graph.number_of_edges(), false),
                             req(graph.number_of_nodes()),
                             active(graph.number_of_nodes(), false),
                             in_S(graph.number_of_nodes(), false),
                             can_scan_out(graph.number_of_nodes(), true) {}
                            //  is_sorted(graph.number_of_nodes(), false) {}

void NewVariant::reset(vector<int> &next_index) {
    next_index.assign(graph.number_of_nodes(), 0);
}

const vector<EdgeId>& NewVariant::get_requested_neighbors(NodeId nodeId) const {
    return req[nodeId];
}

EdgeId NewVariant::next(vector<int> &next_index, NodeId &nodeId, const vector<EdgeId>& adjacent_neighbor_edges) {
    if (nodeId < 0 || nodeId >= graph.number_of_nodes()) return INVALID_EDGE;

    if (next_index[nodeId] < (int)adjacent_neighbor_edges.size()) {
        return adjacent_neighbor_edges[next_index[nodeId]++]; // return current, then advance
    }
    return INVALID_EDGE;
}

void NewVariant::forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq) {
    EdgeId edgeId = INVALID_EDGE;

    // if there are still out-pertinent edges to scan
    if (can_scan_out[nodeId]) {
       edgeId = next(next_index_out_adj, nodeId, graph.get_out_neighbors(nodeId));
       if (edgeId == INVALID_EDGE) {
            // no more out-pertinent edges
           can_scan_out[nodeId] = false; 
       } else if (edgeId != INVALID_EDGE) {
            const Edge& edge = graph.get_edge(edgeId);
            if (isfinite(median_distance) && isfinite(cost[edge.src])) {

             if (edge.cost > 2 * (median_distance - cost[edge.src])) {
                can_scan_out[nodeId] = false; 
            }
        }
       }
    }

    // if no out-pertinent edges to scan, check requested in-pertinent edges
    if (!can_scan_out[nodeId]) {
        edgeId = next(next_index_request, nodeId, get_requested_neighbors(nodeId));
    }

    if (edgeId != INVALID_EDGE) {
        // if apprpriate edge is found
        active[nodeId] = true;
    } else {
        // if no appropriate edge is found 
        active[nodeId] = false;
    }

    if (active[nodeId]) {
       const Edge& edge = graph.get_edge(edgeId);
       if (isfinite(cost[edge.src])) {
        pq.emplace(cost[edge.src] + edge.cost, edgeId);
       }

    }
}

void NewVariant::backward(NodeId nodeId, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq) {
    //make sure v is not in S.
    EdgeId edgeId = next(next_index_in_adj, nodeId, graph.get_in_neighbors(nodeId));
    if (edgeId != INVALID_EDGE) {
        const Edge& edge = graph.get_edge(edgeId); // (u,v)
        pq.emplace(edge.cost, edgeId);
    }
}

void NewVariant::append_to_request(NodeId nodeId, EdgeId edgeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq) {
    bool index_past_end = next_index_request[nodeId] >= req[nodeId].size();
    req[nodeId].push_back(edgeId);

    // for (EdgeId eid : req[nodeId]) {
    //     const auto& edge = graph.get_edge(eid);
        // cout << "edge.trg: " << edge.trg << endl;
    // }

    if (index_past_end) {
        next_index_request[nodeId] = req[nodeId].size() - 1;
    }

    //urgent request
    if (in_S[nodeId] && !active[nodeId]) {
        forward(nodeId, cost , pq);
    }
}

//count edge relaxation in dijkstra.
SsspResult NewVariant::compute_shortest_path(NodeId src, NodeId dst)
{
    int number_of_nodes = graph.number_of_nodes();
    int num_of_pops = 0;

    // 1. Initialize distances and priority queue.
    vector<Cost> cost(number_of_nodes, INF_COST);
    vector<NodeId> prev(number_of_nodes, -1);
    vector<EdgeId> via_edge(number_of_nodes, -1);

    req.clear();
    req.resize(graph.number_of_nodes());
    active.assign(graph.number_of_nodes(), false);
    in_S.assign(graph.number_of_nodes(), false);
    can_scan_out.assign(graph.number_of_nodes(), true);
    
    // Initialise the threshold 
    median_distance = INF_COST;
    // median_distance = -1;

    // Check for trivial case
    if (src == dst)
    {
        cost[src] = 0;
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
    if (src < 0 || src >= number_of_nodes || dst >= number_of_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    // priority queue P is analogous to the priority queue used by Spira’s algorithm
    priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>> P;
    
    //  second priority queue Q is used to identify in-pertinent edges
    priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>> Q;

    // Initialize the source node
    reset(next_index_out_adj);
    reset(next_index_in_adj);
    reset(next_index_request);
    
    cost[src] = 0;
    in_S[src] = true;
    forward(src, cost, P);
    int settled = 1;

    // While the queue is not empty:
    while (P.size() > 0 && settled < number_of_nodes)
    {
        // Extract the node with the smallest distance.
        auto [curr_cost, edgeId] = P.top();
        P.pop();
        num_of_pops += 1;

        const Edge& edge = graph.get_edge(edgeId);
        NodeId node = edge.src;

        // Is this check needed?
        // if (curr_cost !=  cost[node] + edge.cost)
        // continue;

        const Cost EPS = (Cost)1e-9;  // use 1e-6 if weights ~1
        if (!std::isfinite(cost[node]) || !std::isfinite(curr_cost) ||
            curr_cost > cost[node] + edge.cost + EPS) {
            continue; // stale/invalid
        }

        //call forward(u)
        forward(node, cost, P);

        // if v not in S
        NodeId trg = edge.trg;
        if(!in_S[trg]) {
            cost[trg] = curr_cost;
            prev[trg] = node;
            via_edge[trg] = edgeId;
            in_S[trg] = true;
            ++settled;

            // Early exit when destination finalized
            if (dst >= 0 && trg == dst)
                return build_path(prev, cost, via_edge, dst, num_of_pops);

            forward(trg, cost, P);

            if (settled == (number_of_nodes + 1) / 2){
                median_distance = cost[trg];
                // backward scan the first incoming edge of each vertex not yet in S
                // and insert it into the priority queue Q
                for (const Node& node : graph.get_nodes()) {
                    if (!in_S[node.id]){
                        backward(node.id, Q);
                    }

                }
            }
        }

        // Find more in-pertinent edges
        while (Q.size() > 0 ) {
            Cost minP = (P.empty()) ? INF_COST : (P.top().first);

            if (minP != INF_COST && Q.top().first >= 2 * (minP - median_distance)) {
                break; // condition no longer holds
            }

            auto [in_cost, in_edgeId] = Q.top();
            Q.pop();

            
            const Edge& edge = graph.get_edge(in_edgeId);
            
            if (!in_S[edge.trg]) {
                backward(edge.trg, Q);
                append_to_request(edge.src, in_edgeId, cost, P);
            }  // already in S
            
        }
        
    }

     // If dst == -1, build full shortest path tree
    if (dst == -1) {
        return build_path(prev, cost, via_edge, dst, num_of_pops);
    }

    // No path is found.
    else if (cost[dst] < INF_COST)
        return build_path(prev, cost, via_edge, dst, num_of_pops);
    else
        return SsspResult{{}, -1, {}, {}};
}

SsspResult NewVariant::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops)
{
    SsspResult result;
    result.number_of_pops = num_of_pops;

    result.avg_pops_per_node =
                static_cast<double>(num_of_pops) / static_cast<double>(graph.number_of_edges());

    if (dst < 0 || dst >= (int)prev.size() || cost[dst] == INF_COST)
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
