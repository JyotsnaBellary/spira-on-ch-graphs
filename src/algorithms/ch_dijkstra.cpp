#include <algorithms/ch_dijkstra.hpp>
// #include <algorithms/dijkstra.hpp>

CH_Dijkstra::CH_Dijkstra(const CH_Graph& graph): graph(graph) {}


//turn this into bidirectional djikstra 
DijkstraResult CH_Dijkstra::compute_shortest_path(NodeId src, NodeId dst) {
    int num_nodes = graph.num_nodes();

    // 1. Initialize for forward search
    vector<Dist> dist_forward(num_nodes, INF);
    vector<int> prev_forward(num_nodes, -1);
    vector<EdgeId> viaEdge_forward(num_nodes, -1);

    //priority queue
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

    //define a best_node where backward and forward meet
    NodeId best_node = -1;
    Dist best_dist = INF;

    //not needed. 
    vector<NodeId> visited_forward(num_nodes);
    vector<NodeId> visited_backward(num_nodes);

    // Check for trivial case
    // if (src == dst) {
    //     dist[src] = 0;
    //     return DijkstraResult{.path = {src}, .total_cost = 0, .edge_ids = {}};
    // }


    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes) {
        std::cerr << "Error: invalid source/destination node\n";
        std::exit(EXIT_FAILURE);   // exit(1) also fine
    }

    // While the queue is not empty:
    while(pq_forward.size() > 0 || pq_backward.size() > 0) {

        if (pq_forward.size() > 0) {
            // Extract the node with the smallest distance for forward search
            auto [curr_dist, node] = pq_forward.top();
            pq_forward.pop();

            // Check if this node was visited in backward search
            if (visited_backward[node]) {
                Dist total_dist = dist_forward[node] + dist_backward[node];
                if (total_dist < best_dist) {
                    best_dist = total_dist;
                    best_node = node;
                }
            }

            ///is this wrong
            // also might crash here if pq_backward is empty
            if (dist_forward[node] + pq_backward.top().first >= best_dist) {
                return build_path(prev_forward, dist_forward, viaEdge_forward, best_node);
            }

            // If we have already found a better path, skip this one.
            // is this correct?
            if (curr_dist != dist_forward[node]) continue;

            // // Mark node as visited in forward search
            // visited_forward[node] = true;

            if (!visited_forward[node]) {
                visited_forward[node] = true;
            
                // Explore neighbors in forward search
                for (auto& [adjacent_node, edgeInfo] : graph.up_neighbors(node)) {
                    Dist new_dist = dist_forward[node] + static_cast<Dist>(edgeInfo.second);
                    if (new_dist < dist_forward[adjacent_node]) {
                        dist_forward[adjacent_node] = new_dist;
                        prev_forward[adjacent_node] = node;
                        viaEdge_forward[adjacent_node] = edgeInfo.first;
                        pq_forward.push({new_dist, adjacent_node});
                    }
                }
            }
        }

        if (pq_backward.size() > 0) {

            // Extract the node with the smallest distance for backward search
            auto [curr_dist, node] = pq_backward.top();
            pq_backward.pop();

            // check if this node was visited in forward search
            if (visited_forward[node]) {
                Dist total_dist = dist_forward[node] + dist_backward[node];
                if (total_dist < best_dist) {
                    best_dist = total_dist;
                    best_node = node;
                }
            }

            if (dist_backward[node] + pq_forward.top().first >= best_dist) {
                return build_path(prev_backward, dist_backward, viaEdge_backward, best_node);
            }

            // Explore neighbors in backward search
            
        }
    }

    // No path is found.
    return DijkstraResult{ {}, -1, {}};
}

DijkstraResult CH_Dijkstra::build_path(const vector<int>& prev, const vector<Dist>& dist, const vector<EdgeId>& viaEdge, NodeId dst) 
{
    DijkstraResult result;
    if (dst < 0 || dst >= (int)prev.size() || dist[dst] >= (long long) (std::numeric_limits<long long>::max()/8)) {
        result.total_cost = -1; // unreachable
        return result;
    }

    // Reconstruct nodes (backwards)
    std::vector<int> rev_nodes;
    std::vector<EdgeId> rev_edges;

    for (int v = dst; v != -1; v = prev[v]) {
        rev_nodes.push_back(v);
        if (prev[v] != -1 && viaEdge[v] != INVALID_EDGE) {
            rev_edges.push_back(viaEdge[v]); // edge used to reach v
        }
    }

    std::reverse(rev_nodes.begin(), rev_nodes.end());
    std::reverse(rev_edges.begin(), rev_edges.end());

    result.path = std::move(rev_nodes);
    result.edge_ids = std::move(rev_edges);
    result.total_cost = dist[dst];  // or keep Dist in the struct

    return result;
}
// vector<NodeId> CH_Dijkstra::find_shortest_path(CH_Graph& graph, NodeId src) {
//     // Implement a bidirectional Dijkstra's algorithm on the CH graph
//     // using upward_adj and downward_adj for the search.
//     // check for the common node 
//     // Return the shortest path as a vector of NodeIds.

//     // Placeholder implementation
//     return {};
// }