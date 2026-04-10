#include <algorithms/ch_dijkstra.hpp>
// #include <algorithms/dijkstra.hpp>

CH_Dijkstra::CH_Dijkstra(const CH_Graph &graph) : graph(graph) {}

//turn this into bidirectional djikstra
CH_DijkstraResult CH_Dijkstra::compute_shortest_path(NodeId src, NodeId dst)
{
    int num_nodes = graph.num_nodes();
    int number_of_pops = 0;

    // 1. Initialize for forward search
    vector<Dist> dist_forward(num_nodes, INF);
    vector<int> prev_forward(num_nodes, -1);
    vector<EdgeId> viaEdge_forward(num_nodes, -1);

    // priority queue
    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq_forward;
    // Initialize the source node
    dist_forward[src] = 0;
    pq_forward.push({0, src});

    // 2. Initialize for backward search
    vector<Dist> dist_backward(num_nodes, INF);
    vector<int> prev_backward(num_nodes, -1);
    vector<EdgeId> viaEdge_backward(num_nodes, -1);

    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq_backward;
    // Initialize the destination node
    dist_backward[dst] = 0;
    pq_backward.push({0, dst});

    // define a best_node where backward and forward meet
    NodeId best_node = -1;
    Dist best_dist = INF;
    bool stop_f = false, stop_b = false;

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    while ((!pq_forward.empty() && !stop_f) || (!pq_backward.empty() && !stop_b))
    {

        bool take_f = !stop_f && !pq_forward.empty() &&
                      (pq_backward.empty() || pq_forward.top().first <= pq_backward.top().first);

        if (take_f)
        {
            //-------------------FORWARD-------------------
            auto [curr_dist_forward, forward_node] = pq_forward.top();
            pq_forward.pop();
            number_of_pops += 1;

            // Check for stale Dist values
            if (curr_dist_forward != dist_forward[forward_node])
                continue;

            if (dist_backward[forward_node] < INF)
            {
                // Update best node
                Dist total = dist_forward[forward_node] + dist_backward[forward_node];
                if (total < best_dist)
                {
                    best_dist = total;
                    best_node = forward_node;
                }
            }

            // exit if current distance exceeds best distance
            if (curr_dist_forward >= best_dist)
            {
                stop_f = true;
                if (stop_b)
                    break; // both sides done
                else
                    continue; // finish backward side
            }

            // Relax upward neighbors
            for (auto &[adjacent_node, edgeInfo] : graph.up_neighbors(forward_node))
            {
                Dist new_dist = dist_forward[forward_node] + (Dist)(edgeInfo.second);
                if (new_dist < dist_forward[adjacent_node])
                {
                    dist_forward[adjacent_node] = new_dist;
                    prev_forward[adjacent_node] = forward_node;
                    viaEdge_forward[adjacent_node] = edgeInfo.first;
                    pq_forward.push({new_dist, adjacent_node});
                }
            }
        }

        else
        {
            //-------------------BACKWARD-------------------
            auto [curr_dist_backward, backward_node] = pq_backward.top();
            pq_backward.pop();
            number_of_pops += 1;

            if (curr_dist_backward != dist_backward[backward_node])
                continue;

            // Update best node
            if (dist_forward[backward_node] < INF)
            {
                Dist total = dist_backward[backward_node] + dist_forward[backward_node];
                if (total < best_dist)
                {
                    best_dist = total;
                    best_node = backward_node;
                }
            }

            // exit if current distance exceeds best distance
            if (curr_dist_backward >= best_dist)
            {
                stop_b = true;
                if (stop_f)
                    break; // both sides done
                else
                    continue; // finish forward side
            }

            // Relax upward neighbors
            for (auto &[adjacent_node, edgeInfo] : graph.up_neighbors(backward_node))
            {
                Dist new_dist = dist_backward[backward_node] + (Dist)(edgeInfo.second);

                if (new_dist < dist_backward[adjacent_node])
                {
                    dist_backward[adjacent_node] = new_dist;
                    prev_backward[adjacent_node] = backward_node;
                    viaEdge_backward[adjacent_node] = edgeInfo.first;
                    pq_backward.push({new_dist, adjacent_node});
                }
            }
        }
    }

    // Finished. If no path:
    if (best_dist >= INF || best_node == -1)
        return CH_DijkstraResult{{}, {}, -1, {}, {}, number_of_pops};
    return build_ch_path(prev_forward, prev_backward, viaEdge_forward, viaEdge_backward, best_node, best_dist, number_of_pops);
}

CH_DijkstraResult CH_Dijkstra::build_ch_path(const vector<int> &prev_forward, const vector<int> &prev_backward, const vector<EdgeId> &viaEdge_forward, const vector<EdgeId> &viaEdge_backward, NodeId best_node, Dist best_dist, int number_of_pops)
{
    CH_DijkstraResult result;
    result.number_of_pops = number_of_pops;

    result.total_cost = best_dist;

    // Reconstruct nodes from Left: src -> ... -> meet
    vector<int> nodes;
    vector<EdgeId> edges;

    vector<int> backward_nodes;
    vector<EdgeId> backward_edges;

    for (int cur = best_node; cur != -1; cur = prev_forward[cur])
        result.ch_path.push_back(cur);

    reverse(result.ch_path.begin(), result.ch_path.end());

    for (size_t i = 1; i < result.ch_path.size(); ++i)
    {
        Edge edge = graph.get_edge(viaEdge_forward[result.ch_path[i]]);
        result.ch_edge_ids.push_back(edge.id);
    }

    for (int cur = best_node; cur != -1; cur = prev_backward[cur])
        backward_nodes.push_back(cur);

    reverse(backward_nodes.begin(), backward_nodes.end());

    for (size_t i = 1; i < backward_nodes.size(); ++i)
    {
        Edge edge = graph.get_edge(viaEdge_backward[backward_nodes[i]]);
        backward_edges.push_back(edge.rev_id);
    }

    // Direct them towards destination
    reverse(backward_edges.begin(), backward_edges.end());
    reverse(backward_nodes.begin(), backward_nodes.end());

    // store in struct
    result.ch_path.insert(result.ch_path.end(), backward_nodes.begin() + 1, backward_nodes.end());
    result.ch_edge_ids.insert(result.ch_edge_ids.end(), backward_edges.begin(), backward_edges.end());

    return result;
}