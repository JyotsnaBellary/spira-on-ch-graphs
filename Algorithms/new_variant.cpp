#include "new_variant.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

NewVariant::NewVariant(Graph &graph) : graph(graph),
                                       next_index_out_adj(graph.number_of_nodes(), 0),
                                       next_index_in_adj(graph.number_of_nodes(), 0),
                                       next_index_request(graph.number_of_nodes(), 0),
                                       in_pertinent_edges(graph.number_of_edges(), false),
                                       out_pertinent_edges(graph.number_of_edges(), false),
                                       in_pertinent_edges_extracted_in_forward_phase(graph.number_of_edges(), false),
                                       req(graph.number_of_nodes()),
                                       active(graph.number_of_nodes(), false),
                                       in_S(graph.number_of_nodes(), false),
                                       can_scan_out(graph.number_of_nodes(), true) {}

void NewVariant::reset(vector<int> &next_index)
{
    next_index.assign(graph.number_of_nodes(), 0);
}

const vector<EdgeId> &NewVariant::get_requested_neighbors(NodeId nodeId) const
{
    return req[nodeId];
}

EdgeId NewVariant::next(vector<int> &next_index, NodeId &nodeId, const vector<EdgeId> &adjacent_neighbor_edges)
{
    if (nodeId < 0 || nodeId >= graph.number_of_nodes())
        return INVALID_EDGE;

    if (next_index[nodeId] < (int)adjacent_neighbor_edges.size())
    {
        return adjacent_neighbor_edges[next_index[nodeId]++]; // return current, then advance
    }
    return INVALID_EDGE;
}

void NewVariant::forward(NodeId nodeId, vector<Cost> &cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>> &pq)
{
    EdgeId edgeId = INVALID_EDGE;

    // if there are still out-pertinent edges to scan
    if (can_scan_out[nodeId])
    {
        edgeId = next(next_index_out_adj, nodeId, graph.get_out_neighbors(nodeId));
        if (edgeId == INVALID_EDGE)
        {
            // no more outgoing edges left to scan for this node
            can_scan_out[nodeId] = false;
        }
        else if (edgeId != INVALID_EDGE)
        {
            const Edge &edge = graph.get_edge(edgeId);
            // Though this edge is valid, it may or may not be out-perminent after the first stage
            // The ones extracted are out pertinent and the remaining n/2 edges may or may not be pertinent,
            // so do not classify as out-pertinent yet

            // Check if we are in the 2nd stage of the algorithm
            if (isfinite(median_distance) && isfinite(cost[edge.src]))
            {
                // Since Median is finite, we are in the 2nd stage of the algorithm
                // Check if the edge is out-pertinent
                if (edge.cost > 2 * (median_distance - cost[edge.src]))
                {
                    // if not, set to false as no more out-pertinent edges will be found
                    can_scan_out[nodeId] = false;
                }
                else
                {   
                    // mark as out-pertinent edge
                    out_pertinent_edges[edgeId] = true;
                }
            }
        }
    }

    // if no out-pertinent edges to scan, check requested in-pertinent edges
    if (!can_scan_out[nodeId])
    {
        edgeId = next(next_index_request, nodeId, get_requested_neighbors(nodeId));
        if (edgeId != INVALID_EDGE)
        {

            // all requested edges are in-pertinent edges
            if (out_pertinent_edges[edgeId])
            {
                //add only in debug mode
                // cerr << "ERROR: edge " << edgeId
                //       << " appears in request list but is marked out-pertinent.\n";
                      if (in_pertinent_edges[edgeId]) {
                          // edge already marked in-pertinent too
                          cerr << "ERROR: edge " << edgeId
                               << " is marked both in-pertinent and out-pertinent.\n";
                      }
            }
            in_pertinent_edges_extracted_in_forward_phase[edgeId] = true;
        }
    }

    if (edgeId != INVALID_EDGE)
    {
        // if apprpriate edge is found
        active[nodeId] = true;
    }
    else
    {
        // if no appropriate edge is found
        active[nodeId] = false;
    }

    if (active[nodeId])
    {
        const Edge &edge = graph.get_edge(edgeId);
        if (isfinite(cost[edge.src]))
        {
            pq.emplace(cost[edge.src] + edge.cost, edgeId);
        }
    }
}

void NewVariant::backward(NodeId nodeId, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>> &pq)
{
    // make sure v is not in S.
    EdgeId edgeId = next(next_index_in_adj, nodeId, graph.get_in_neighbors(nodeId));
    if (edgeId != INVALID_EDGE)
    {
        const Edge &edge = graph.get_edge(edgeId); // (u,v)
        pq.emplace(edge.cost, edgeId);
    }
}

void NewVariant::append_to_request(NodeId nodeId, EdgeId edgeId, vector<Cost> &cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>> &pq)
{
    bool index_past_end = next_index_request[nodeId] >= req[nodeId].size();
    if (edgeId == INVALID_EDGE) return;
    req[nodeId].push_back(edgeId);

    if (index_past_end)
    {
        next_index_request[nodeId] = req[nodeId].size() - 1;
    }

    // urgent request
    if (in_S[nodeId] && !active[nodeId])
    {
        forward(nodeId, cost, pq);
    }
}

// count edge relaxation in dijkstra.
SsspResult NewVariant::compute_shortest_path(NodeId src, NodeId dst)
{
    int number_of_nodes = graph.number_of_nodes();
    int num_of_pops = 0;
    int num_of_Q_pops = 0;
    int number_of_unclassified_edges_set_to_out_pertinent = 0;

    // 1. Initialize distances and priority queue.
    vector<Cost> cost(number_of_nodes, INF_COST);
    vector<NodeId> prev(number_of_nodes, -1);
    vector<EdgeId> via_edge(number_of_nodes, -1);

    req.clear();
    req.resize(graph.number_of_nodes());
    active.assign(graph.number_of_nodes(), false);
    in_S.assign(graph.number_of_nodes(), false);
    can_scan_out.assign(graph.number_of_nodes(), true);

    // reset pertinence state
    out_pertinent_edges.assign(graph.number_of_edges(), false);
    in_pertinent_edges.assign(graph.number_of_edges(), false);
    in_pertinent_edges_extracted_in_forward_phase.assign(graph.number_of_edges(), false);

    // Initialise the threshold
    median_distance = INF_COST;

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
            .number_of_pops = num_of_pops,
            .out_pertinent_edges = out_pertinent_edges,
            .in_pertinent_edges = in_pertinent_edges};
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

        const Edge &edge = graph.get_edge(edgeId);
        NodeId node = edge.src;

        // Section 4
        // In the 1st stage, All extracted edges are out-pertinent edges. 
        // However at the end of first stage, the n/2 edges left in P may or may not be pertinent.
        if (!isfinite(median_distance))
        {
            out_pertinent_edges[edgeId] = true;
        }

        // If in 2nd stage, 
        if (isfinite(median_distance))
        {
            // check if edge was one of the n/2 left in P that remained unclassified as out-pertinent.
            if (!out_pertinent_edges[edgeId] and !in_pertinent_edges[edgeId])
            {
                    number_of_unclassified_edges_set_to_out_pertinent++;

                // We are in the 2nd stage of the algorithm
                // Check if the edge needs to be classified as an out-pertinent edge
                if (edge.cost <= 2 * (median_distance - cost[edge.src]))
                {
                    out_pertinent_edges[edgeId] = true;
                } 
                else if (edge.cost < 2 * (cost[edge.trg] - median_distance))
                {
                    // Edge is in-pertinent
                    in_pertinent_edges[edgeId] = true;

                }               
            }
        }
        

        const Cost EPS = (Cost)1e-9; // use 1e-6 if weights ~1
        if (!std::isfinite(cost[node]) || !std::isfinite(curr_cost) ||
            curr_cost > cost[node] + edge.cost + EPS)
        {
            continue; // stale/invalid
        }

        // call forward(u)
        forward(node, cost, P);

        // if v not in S
        NodeId trg = edge.trg;
        if (!in_S[trg])
        {
            cost[trg] = curr_cost;
            prev[trg] = node;
            via_edge[trg] = edgeId;
            in_S[trg] = true;
            ++settled;

            // Early exit when destination finalized
            if (dst >= 0 && trg == dst)
                return build_path(prev, cost, via_edge, dst, num_of_pops, num_of_Q_pops, median_distance);

            forward(trg, cost, P);

            if (settled == (number_of_nodes + 1) / 2)
            {
                median_distance = cost[trg];
                // backward scan the first incoming edge of each vertex not yet in S
                // and insert it into the priority queue Q
                for (const Node &node : graph.get_nodes())
                {
                    if (!in_S[node.id])
                    {
                        backward(node.id, Q);
                    }
                }
            }
        }

        // Find more in-pertinent edges
        while (Q.size() > 0)
        {
            Cost minP = (P.empty()) ? INF_COST : (P.top().first);

            if (minP != INF_COST && Q.top().first >= 2 * (minP - median_distance))
            {
                break; // condition no longer holds
            }

            auto [in_cost, in_edgeId] = Q.top();
            Q.pop();
            num_of_Q_pops += 1;
            const Edge &edge = graph.get_edge(in_edgeId);

            if (!in_S[edge.trg])
            {
                // All edges extracted from Q that are pushed to Request are in-pertinent edges
                if (out_pertinent_edges[in_edgeId]) {
                    cerr << "ERROR: edge " << in_edgeId
                            << " discovered in Q but already out-pertinent.\n";
                    // optional: std::abort();
                } 
                
                backward(edge.trg, Q);
                append_to_request(edge.src, in_edgeId, cost, P);

            } // already in S
        }
    }

    // If dst == -1, build full shortest path tree
    if (dst == -1)
    {
        return build_path(prev, cost, via_edge, dst, num_of_pops, num_of_Q_pops, median_distance);
    }

    // No path is found.
    else if (cost[dst] < INF_COST)
        return build_path(prev, cost, via_edge, dst, num_of_pops, num_of_Q_pops, median_distance);
    else
        return SsspResult{{}, -1, {}, {}};
}

SsspResult NewVariant::build_path(const vector<int> &prev, const vector<Cost> &cost, const vector<EdgeId> &viaEdge, NodeId dst, int num_of_pops, int num_of_Q_pops, Cost median)
{

    SsspResult result;
    result.number_of_pops = num_of_pops;

    result.avg_pops_per_node =
        static_cast<double>(num_of_pops) / static_cast<double>(graph.number_of_edges());
    result.in_pertinent_edges = in_pertinent_edges;
    result.in_pertinent_edges_extracted_in_forward_phase = in_pertinent_edges_extracted_in_forward_phase;
    result.out_pertinent_edges = out_pertinent_edges;
    result.median = median;
    if (dst < 0)
    {

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
