#include <shortest-path/bi-directional/bi-directional-spira.hpp>
#include <algorithm>
#include <iostream>
// #include <types.hpp>

Bi_Directional_Spira::Bi_Directional_Spira(CH_Graph& graph)
    : graph(graph),
      next_index_forward(graph.number_of_nodes(), 0),
      next_index_backward(graph.number_of_nodes(), 0) {}

void Bi_Directional_Spira::reset()
{
    next_index_forward.assign(graph.number_of_nodes(), 0);
    next_index_backward.assign(graph.number_of_nodes(), 0);
}

EdgeId Bi_Directional_Spira::next(NodeId nodeId, vector<int>& next_index)
{
    if (nodeId < 0 || nodeId >= graph.number_of_nodes())
        return INVALID_EDGE;

    const vector<EdgeId>& adjacent_neighbor_edges = graph.get_out_neighbors(nodeId);

    if (next_index[nodeId] < static_cast<int>(adjacent_neighbor_edges.size()))
        return adjacent_neighbor_edges[next_index[nodeId]++];

    return INVALID_EDGE;
}

void Bi_Directional_Spira::forward(
    NodeId nodeId,
    const vector<Cost>& cost,
    vector<int>& next_index,
    EdgePQ& pq)
{
    EdgeId edgeId = next(nodeId, next_index);
    if (edgeId == INVALID_EDGE)
        return;

    const Edge& edge = graph.get_edge(edgeId);
    pq.emplace(cost[edge.src] + edge.cost, edgeId);
}

// Returns direction that maintains min-key selection
bool Bi_Directional_Spira::should_expand_forward(
    const EdgePQ& pq_forward,
    const EdgePQ& pq_backward,
    bool stop_f,
    bool stop_b) const
{
    bool can_f = !stop_f && !pq_forward.empty();
    bool can_b = !stop_b && !pq_backward.empty();

    return can_f && (!can_b || pq_forward.top().first <= pq_backward.top().first);
}

Bi_Directional_Dijkstra_Result Bi_Directional_Spira::compute_shortest_path(NodeId src, NodeId dst)
{
    int num_nodes = graph.number_of_nodes();
    int number_of_pops = 0;

    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE);
    }

    if (src == dst)
    {
        return Bi_Directional_Dijkstra_Result{
            .path = {},
            .ch_path = {src},
            .total_cost = 0,
            .edge_ids = {},
            .ch_edge_ids = {},
            .number_of_pops = 0};
    }

    vector<Cost> cost_forward(num_nodes, INF_COST);
    vector<NodeId> prev_forward(num_nodes, -1);
    vector<EdgeId> via_edge_forward(num_nodes, INVALID_EDGE);
    vector<bool> settled_forward(num_nodes, false);

    vector<Cost> cost_backward(num_nodes, INF_COST);
    vector<NodeId> prev_backward(num_nodes, -1);
    vector<EdgeId> via_edge_backward(num_nodes, INVALID_EDGE);
    vector<bool> settled_backward(num_nodes, false);

    EdgePQ pq_forward;
    EdgePQ pq_backward;

    reset();

    cost_forward[src] = 0;
    settled_forward[src] = true;
    forward(src, cost_forward, next_index_forward, pq_forward);

    cost_backward[dst] = 0;
    settled_backward[dst] = true;
    forward(dst, cost_backward, next_index_backward, pq_backward);

    NodeId best_node = -1;
    Dist best_dist = INF;
    bool stop_f = false;
    bool stop_b = false;

    while ((!pq_forward.empty() && !stop_f) || (!pq_backward.empty() && !stop_b))
    {
        bool take_f = should_expand_forward(pq_forward, pq_backward, stop_f, stop_b);

        if (take_f)
        {
            auto [curr_cost, edgeId] = pq_forward.top();
            pq_forward.pop();
            number_of_pops += 1;

            const Edge& edge = graph.get_edge(edgeId);
            NodeId node = edge.src;
            NodeId trg = edge.trg;

            if (curr_cost != cost_forward[node] + edge.cost)
                continue;

            forward(node, cost_forward, next_index_forward, pq_forward);

            if (!settled_forward[trg])
            {
                cost_forward[trg] = curr_cost;
                prev_forward[trg] = node;
                via_edge_forward[trg] = edgeId;
                settled_forward[trg] = true;

                if (cost_backward[trg] < INF_COST)
                {
                    Dist total = cost_forward[trg] + cost_backward[trg];
                    if (total < best_dist)
                    {
                        best_dist = total;
                        best_node = trg;
                    }
                }

                forward(trg, cost_forward, next_index_forward, pq_forward);
            }

            if (curr_cost >= best_dist)
                stop_f = true;
        }
        else
        {
            auto [curr_cost, edgeId] = pq_backward.top();
            pq_backward.pop();
            number_of_pops += 1;

            const Edge& edge = graph.get_edge(edgeId);
            NodeId node = edge.src;
            NodeId trg = edge.trg;

            if (curr_cost != cost_backward[node] + edge.cost)
                continue;

            forward(node, cost_backward, next_index_backward, pq_backward);

            if (!settled_backward[trg])
            {
                cost_backward[trg] = curr_cost;
                prev_backward[trg] = node;
                via_edge_backward[trg] = edgeId;
                settled_backward[trg] = true;

                if (cost_forward[trg] < INF_COST)
                {
                    Dist total = cost_backward[trg] + cost_forward[trg];
                    if (total < best_dist)
                    {
                        best_dist = total;
                        best_node = trg;
                    }
                }

                forward(trg, cost_backward, next_index_backward, pq_backward);
            }

            if (curr_cost >= best_dist)
                stop_b = true;
        }
    }

    if (best_dist >= INF || best_node == -1)
        return Bi_Directional_Dijkstra_Result{{}, {}, -1, {}, {}, number_of_pops};

    return build_ch_path(
        prev_forward,
        prev_backward,
        via_edge_forward,
        via_edge_backward,
        best_node,
        best_dist,
        number_of_pops);
}

Bi_Directional_Dijkstra_Result Bi_Directional_Spira::build_ch_path(
    const vector<NodeId>& prev_forward,
    const vector<NodeId>& prev_backward,
    const vector<EdgeId>& via_edge_forward,
    const vector<EdgeId>& via_edge_backward,
    NodeId best_node,
    Dist best_dist,
    int number_of_pops)
{
    Bi_Directional_Dijkstra_Result result;
    result.number_of_pops = number_of_pops;
    result.total_cost = best_dist;

    vector<NodeId> backward_nodes;
    vector<EdgeId> backward_edges;

    for (NodeId cur = best_node; cur != -1; cur = prev_forward[cur])
        result.ch_path.push_back(cur);

    reverse(result.ch_path.begin(), result.ch_path.end());

    for (size_t i = 1; i < result.ch_path.size(); ++i)
        result.ch_edge_ids.push_back(via_edge_forward[result.ch_path[i]]);

    for (NodeId cur = best_node; cur != -1; cur = prev_backward[cur])
        backward_nodes.push_back(cur);

    reverse(backward_nodes.begin(), backward_nodes.end());

    for (size_t i = 1; i < backward_nodes.size(); ++i)
    {
        const Edge& edge = graph.get_edge(via_edge_backward[backward_nodes[i]]);
        backward_edges.push_back(edge.rev_id);
    }

    reverse(backward_edges.begin(), backward_edges.end());
    reverse(backward_nodes.begin(), backward_nodes.end());

    result.ch_path.insert(result.ch_path.end(), backward_nodes.begin() + 1, backward_nodes.end());
    result.ch_edge_ids.insert(result.ch_edge_ids.end(), backward_edges.begin(), backward_edges.end());

    return result;
}
