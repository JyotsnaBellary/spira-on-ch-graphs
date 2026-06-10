#include <shortest-path/bi-directional/bi-directional-dijkstra.hpp>
// #include <algorithms/dijkstra.hpp>

Bi_Directional_Dijkstra::Bi_Directional_Dijkstra(const CH_Graph &graph) : graph(graph) {}

Bi_Directional_Dijkstra::Bi_Directional_Dijkstra(
    const CH_Graph &graph,
    const EliminationTree &elimination_tree)
    : graph(graph),
      elimination_tree(&elimination_tree) {}


bool Bi_Directional_Dijkstra::take_forward(
    SearchMode mode,
    NodeScheduler& forward_scheduler,
    NodeScheduler& backward_scheduler,
    bool stop_f,
    bool stop_b,
    bool& expand_forward_next)
{
    bool can_f = !stop_f && forward_scheduler.has_next();
    bool can_b = !stop_b && backward_scheduler.has_next();

    if (mode == SearchMode::PriorityQueue)
    {
        return can_f &&
               (!can_b ||
                forward_scheduler.peek_cost() <= backward_scheduler.peek_cost());
    }

    if (can_f && can_b)
    {
        bool take_f = expand_forward_next;
        expand_forward_next = !expand_forward_next;
        return take_f;
    }

    return can_f;
}

// turn this into bidirectional djikstra
Bi_Directional_Dijkstra_Result Bi_Directional_Dijkstra::compute_shortest_path(NodeId src, NodeId dst, SearchMode mode)
{
    
    int num_nodes = graph.number_of_nodes();
    int number_of_pops = 0;

    if (mode == SearchMode::EliminationTree &&
        elimination_tree == nullptr)
    {
        throw runtime_error(
            "Elimination tree mode selected but no elimination tree provided.");
    }

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes)
    {
        cerr << "Error: invalid source/destination node\n";
        exit(EXIT_FAILURE); // exit(1) also fine
    }

    // 1. Initialize for forward search
    vector<Dist> dist_forward(num_nodes, INF);
    vector<int> prev_forward(num_nodes, -1);
    vector<EdgeId> viaEdge_forward(num_nodes, -1);

    // Initialise priority queue
    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq_forward;
    priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq_backward;

    NodeScheduler forward_scheduler =
        (mode == SearchMode::PriorityQueue)
            ? NodeScheduler(pq_forward)
            : NodeScheduler(*elimination_tree, src);

    NodeScheduler backward_scheduler =
        (mode == SearchMode::PriorityQueue)
            ? NodeScheduler(pq_backward)
            : NodeScheduler(*elimination_tree, dst);

    // Initialize the source node
    dist_forward[src] = 0;
    forward_scheduler.push(0, src);

    // 2. Initialize for backward search
    vector<Dist> dist_backward(num_nodes, INF);
    vector<int> prev_backward(num_nodes, -1);
    vector<EdgeId> viaEdge_backward(num_nodes, -1);

    // Initialize the destination node
    dist_backward[dst] = 0;
    backward_scheduler.push(0, dst);

    // define a best_node where backward and forward meet
    NodeId best_node = -1;
    Dist best_dist = INF;
    bool stop_f = false, stop_b = false;

    

    bool expand_forward_next = true;
    while ((forward_scheduler.has_next() && !stop_f) || (backward_scheduler.has_next() && !stop_b))
    {

        // bool take_f = !stop_f && forward_scheduler.has_next() &&
        //               (!forward_scheduler.has_next() || forward_scheduler.peek_cost() <= backward_scheduler.peek_cost());

    //     bool take_f;

    //     if (mode == SearchMode::PriorityQueue)
    //     {
    //         take_f =
    //             !stop_f &&
    //             forward_scheduler.has_next() &&
    //             (!backward_scheduler.has_next() ||
    //              forward_scheduler.peek_cost() <= backward_scheduler.peek_cost());
    //     }
    //     else
    //     {
    //         if (forward_scheduler.has_next() && backward_scheduler.has_next())
    // {
    //     take_f = expand_forward_next;
    //     expand_forward_next = !expand_forward_next;
    // }
    // else
    // {
    //     take_f = forward_scheduler.has_next();
    // }
    //     }

    bool take_f = take_forward(
    mode,
    forward_scheduler,
    backward_scheduler,
    stop_f,
    stop_b,
    expand_forward_next);

        if (take_f)
        {
            cout << "taking forward" << endl;
            //-------------------FORWARD-------------------
            auto [curr_dist_forward, forward_node] = forward_scheduler.pop(dist_forward);
            number_of_pops += 1;

            cout << "popped forward node: " << forward_node << " with dist: " << curr_dist_forward << endl;
            // Check for stale Dist values
            if (mode == SearchMode::PriorityQueue && curr_dist_forward != dist_forward[forward_node]) {
                 cout << "stale" << endl;
                continue;
            }
               

            // if (dist_backward[forward_node] == INF)
            // {
            //     cout << "forward node not reachable from backward" << endl;
            //     continue;
            // }

            if (dist_backward[forward_node] < INF)
            {
                // Update best node
                Dist total = dist_forward[forward_node] + dist_backward[forward_node];
                if (total < best_dist)
                {
                    cout << "best node updated: " << forward_node << " with dist: " << total << endl;
                    best_dist = total;
                    best_node = forward_node;
                }
            }

            

            // exit if current distance exceeds best distance
            if (mode == SearchMode::PriorityQueue && curr_dist_forward >= best_dist)
            {
                stop_f = true;
                cout << "finish forward side: " << endl;
                if (stop_b) {
                    cout << "both sides done, breaking" << endl;
                    break; // both sides done
                }
                else {
                    cout << "finish backward side" << endl;
                    continue; // finish backward side
                }
            }
            cout << "relaxing forward node: " << endl;

            // Relax upward neighbors
            for (EdgeId edgeId : graph.get_out_neighbors(forward_node))
            {
                cout << "relaxing forward edge: " << edgeId << endl;
                const Edge& edge = graph.get_edge(edgeId);
                NodeId adjacent_node = edge.trg;
                Cost new_dist = dist_forward[forward_node] + (Dist)(edge.cost);
                if (new_dist < dist_forward[adjacent_node])
                {
                    dist_forward[adjacent_node] = new_dist;
                    prev_forward[adjacent_node] = forward_node;
                    viaEdge_forward[adjacent_node] = edgeId;
                    // pq_forward.push({new_dist, adjacent_node});
                    cout << "pushing to forward scheduler: node " << adjacent_node << " with dist: " << new_dist << endl;
                    forward_scheduler.push(new_dist, adjacent_node);
                }
            }
        }

        else
        {
            //-------------------BACKWARD-------------------
            auto [curr_dist_backward, backward_node] = backward_scheduler.pop(dist_backward);
            // pq_backward.pop();
            number_of_pops += 1;

            cout << "popping backward node " << backward_node << " with dist: " << curr_dist_backward << endl;
            if (mode == SearchMode::PriorityQueue && curr_dist_backward != dist_backward[backward_node]) {
                cout << "stale" << endl;
                continue;

            }

            // if (dist_forward[backward_node] == INF) {
            //     cout << "backward node not reachable from forward" << endl;
            //     continue;
            // }

            // Update best node
            if (dist_forward[backward_node] < INF)
            {
                Dist total = dist_backward[backward_node] + dist_forward[backward_node];
                if (total < best_dist)
                {
                    cout << "best node updated: " << backward_node << " with dist: " << total << endl;
                    best_dist = total;
                    best_node = backward_node;
                }
            }

            // exit if current distance exceeds best distance
            if (mode == SearchMode::PriorityQueue && curr_dist_backward >= best_dist)
            {
                stop_b = true;
                if (stop_f) {
                    cout << "both sides done, breaking" << endl;
                    break; // both sides done

                }
                else {
                    cout << "finish forward side" << endl;
                    continue; // finish forward side
                }
            }

            // Relax upward neighbors
            for (EdgeId edgeId : graph.get_out_neighbors(backward_node))
            {
                const Edge& edge = graph.get_edge(edgeId);
            NodeId adjacent_node = edge.trg;
                Cost new_dist = dist_backward[backward_node] + edge.cost;

                if (new_dist < dist_backward[adjacent_node])
                {
                    dist_backward[adjacent_node] = new_dist;
                    prev_backward[adjacent_node] = backward_node;
                    viaEdge_backward[adjacent_node] = edgeId;
                    // pq_backward.push({new_dist, adjacent_node});
                    backward_scheduler.push(new_dist, adjacent_node);
                    cout << "pushing to backward scheduler: node " << adjacent_node << " with dist: " << new_dist << endl;
                }
            }
        }
    }

    // Finished. If no path:
    if (best_dist >= INF || best_node == -1){
        cout << "No path found between " << src << " and " << dst << endl;
        return Bi_Directional_Dijkstra_Result{{}, {}, -1, {}, {}, number_of_pops};
    }
    return build_ch_path(prev_forward, prev_backward, viaEdge_forward, viaEdge_backward, best_node, best_dist, number_of_pops);
}

Bi_Directional_Dijkstra_Result Bi_Directional_Dijkstra::build_ch_path(const vector<int> &prev_forward, const vector<int> &prev_backward, const vector<EdgeId> &viaEdge_forward, const vector<EdgeId> &viaEdge_backward, NodeId best_node, Dist best_dist, int number_of_pops)
{
    Bi_Directional_Dijkstra_Result result;
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