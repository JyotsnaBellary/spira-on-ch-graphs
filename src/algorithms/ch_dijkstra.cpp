#include <algorithms/ch_dijkstra.hpp>
// #include <algorithms/dijkstra.hpp>

CH_Dijkstra::CH_Dijkstra(const CH_Graph& graph): graph(graph) {}

//turn this into bidirectional djikstra 
// CH_DijkstraResult CH_Dijkstra::compute_shortest_path(NodeId src, NodeId dst) {
//     int num_nodes = graph.num_nodes();

    
//     // 1. Initialize for forward search
//     vector<Dist> dist_forward(num_nodes, INF);
//     vector<int> prev_forward(num_nodes, -1);
//     vector<EdgeId> viaEdge_forward(num_nodes, -1);

//     //priority queue
//     priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq_forward;
//     // Initialize the source node
//     dist_forward[src] = 0;
//     pq_forward.push({0, src});
   
//     // 2. Initialize for backward search
//     vector<Dist> dist_backward(num_nodes, INF);
//     vector<int> prev_backward(num_nodes, -1);
//     vector<EdgeId> viaEdge_backward(num_nodes, -1);

//     priority_queue<pair<Dist, NodeId>, vector<pair<Dist, NodeId>>, greater<pair<Dist, NodeId>>> pq_backward;
//     // Initialize the destination node
//     dist_backward[dst] = 0;
//     pq_backward.push({0, dst});

//     //define a best_node where backward and forward meet
//     NodeId best_node = -1;
//     Dist best_dist = INF;
//     bool stop_f = false, stop_b = false;

//     // Check for valid node IDs
//     if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes) {
//         std::cerr << "Error: invalid source/destination node\n";
//         std::exit(EXIT_FAILURE);   // exit(1) also fine
//     }

//     while ((!pq_forward.empty() && !stop_f) || (!pq_backward.empty() && !stop_b)) {
//         // Forward search step// choose side to expand (smaller min-key). Safe to peek *your own* queue.
//         // Optional cleaner stop condition:

//         //but is this necessary?
//         // Dist lb_f = pq_forward.empty()  ? INF : pq_forward.top().first;
//         // Dist lb_b = pq_backward.empty() ? INF : pq_backward.top().first;
//         // if (std::min(lb_f, lb_b) >= best_dist) {
//         //     cout << "Both searches exceeded best known distance. Stopping." << endl;
//         //     break;
//         // }

//         bool take_f = !stop_f && !pq_forward.empty() &&
//                     ( pq_backward.empty() || pq_forward.top().first <= pq_backward.top().first );

//         if (take_f) {
//             //-------------------FORWARD-------------------
//             auto [curr_dist_forward, forward_node] = pq_forward.top();
//             pq_forward.pop();

//             // cout << "Forward node: " << forward_node << ", Dist: " << curr_dist_forward << endl;

//             if (curr_dist_forward != dist_forward[forward_node]) continue;

//             if (dist_backward[forward_node] < INF) {
//                 Dist total = dist_forward[forward_node] + dist_backward[forward_node];
//                 if (total < best_dist) {
//                     best_dist = total;
//                     best_node = forward_node;
//                 }
//             }

//             //do we not need this cjeck?
//             if (curr_dist_forward >= best_dist) {
//                 cout << "Forward search exceeded best known distance. Stopping." << endl;
//                 stop_f = true;
//                 if (stop_b) break; // both sides done
//                 else continue;      // finish backward side
//             }

//             cout << "Relaxing Neighbors" << endl;
//             //Relax upward neighbors
//             for (auto& [adjacent_node, edgeInfo] : graph.up_neighbors(forward_node)) {
//                 Dist new_dist = dist_forward[forward_node] + (Dist)(edgeInfo.second);
//                 cout << "Relaxing edge: " << edgeInfo.first << " to node: " << adjacent_node << ", New Dist: " << new_dist << endl;
//                 if (new_dist < dist_forward[adjacent_node]) {
//                     //check this edge, if its a shortcut, just append it to dist forward, prev forward should be same forward node 
//                     //add via edge properly
//                     //push both nodes to pq_forward
//                     if (graph.get_edge(edgeInfo.first).shortcut) {
//                         Edge edge = graph.get_edge(edgeInfo.first);
//                         // cost should be os uv or vu, one of them
//                         dist_forward[edge.sc.middle] = dist_forward[forward_node] + (Dist)(edge.cost);
//                         //this id as well, it should be the edge of the id to the middle node
//                         prev_forward[edge.sc.middle] = edge.id;
//                         pq_forward.push({dist_forward[edge.sc.middle], edge.sc.middle});
//                     }
//                     dist_forward[adjacent_node] = new_dist;
//                     prev_forward[adjacent_node] = forward_node;
//                     viaEdge_forward[adjacent_node] = edgeInfo.first;
//                     pq_forward.push({new_dist, adjacent_node});
//                     cout << "Pushed to forward PQ: " << adjacent_node << ", Dist: " << new_dist << endl;
//                 }
//             }

//         }

//         else {
//             //-------------------BACKWARD-------------------
//             auto [curr_dist_backward, backward_node] = pq_backward.top();
//             pq_backward.pop();

//             cout << "Backward node: " << backward_node << ", Dist: " << curr_dist_backward << endl;

//             if (curr_dist_backward != dist_backward[backward_node]) continue;

//             if (dist_forward[backward_node] < INF) {
//                 Dist total = dist_backward[backward_node] + dist_forward[backward_node];
//                 if (total < best_dist) {
//                     best_dist = total;
//                     best_node = backward_node;
//                 }
//             }

//             // not needed?
//             if (curr_dist_backward >= best_dist) {
//                 cout << "Backward search exceeded best known distance. Stopping." << endl;
//                 stop_b = true;
//                 if (stop_f) break; // both sides done
//                 else continue;      // finish forward side
//             }

//             //Relax upward neighbors
//             for (auto& [adjacent_node, edgeInfo] : graph.up_neighbors(backward_node)) {
//                 Dist new_dist = dist_backward[backward_node] + (Dist)(edgeInfo.second);
//                    const EdgeId edgeId_up  = edgeInfo.first;                  // v->x (up)
//                     const EdgeId edgeId_rev = graph.get_edge(edgeId_up).rev_id;    // x->v (actual) <<< flip

//                     cout << "Relaxing edge: " << edgeId_up << " to node: " << adjacent_node << ", New Dist: " << new_dist << endl;
//                 if (new_dist < dist_backward[adjacent_node]) {
//                     if (graph.get_edge(edgeInfo.first).shortcut) {
//                         Edge edge = graph.get_edge(edgeInfo.first);
//                         dist_backward[edge.sc.middle] = dist_backward[backward_node] + (Dist)(edge.cost);
//                         prev_forward[edge.sc.middle] = edge.rev_id;
//                         pq_forward.push({dist_forward[edge.sc.middle], edge.sc.middle});
//                     }
//                     dist_backward[adjacent_node] = new_dist;
//                     prev_backward[adjacent_node] = backward_node;
//                     viaEdge_backward[adjacent_node] = edgeId_rev;
//                     pq_backward.push({new_dist, adjacent_node});
//                     cout << "Pushed to backward PQ: " << adjacent_node << ", Dist: " << new_dist << endl;
//                 }
//             }

//         }

//     }

//     // cout << "Backward search finished. Best node: " << best_node << ", Best dist: " << best_dist << endl;
//     // cout << "ViaEdge_Forward:" << endl;
//     // for (const auto& edgeId : viaEdge_forward) {
//     //     cout << "Edge ID: " << edgeId << endl;
//     // }

//     // cout << "ViaEdge_Backward:" << endl;
//     // for (const auto& edgeId : viaEdge_backward) {
//     //     cout << "Edge ID: " << edgeId << endl;
//     // }

//     // Finished. If no path:
//     if (best_dist >= INF || best_node == -1) return CH_DijkstraResult{ {}, {}, -1, {}, {} };
//     return build_ch_path(prev_forward, prev_backward, viaEdge_forward, viaEdge_backward, best_node, best_dist);
// }

// //turn this into bidirectional djikstra 
CH_DijkstraResult CH_Dijkstra::compute_shortest_path(NodeId src, NodeId dst) {
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
    bool stop_f = false, stop_b = false;

    // Check for valid node IDs
    if (src < 0 || src >= num_nodes || dst < 0 || dst >= num_nodes) {
        std::cerr << "Error: invalid source/destination node\n";
        std::exit(EXIT_FAILURE);   // exit(1) also fine
    }

    while ((!pq_forward.empty() && !stop_f) || (!pq_backward.empty() && !stop_b)) {
        // Forward search step// choose side to expand (smaller min-key). Safe to peek *your own* queue.
        // Optional cleaner stop condition:

        //but is this necessary?
        // Dist lb_f = pq_forward.empty()  ? INF : pq_forward.top().first;
        // Dist lb_b = pq_backward.empty() ? INF : pq_backward.top().first;
        // if (std::min(lb_f, lb_b) >= best_dist) {
        //     cout << "Both searches exceeded best known distance. Stopping." << endl;
        //     break;
        // }

        bool take_f = !stop_f && !pq_forward.empty() &&
                    ( pq_backward.empty() || pq_forward.top().first <= pq_backward.top().first );

        if (take_f) {
            //-------------------FORWARD-------------------
            auto [curr_dist_forward, forward_node] = pq_forward.top();
            pq_forward.pop();

            // cout << "Forward node: " << forward_node << ", Dist: " << curr_dist_forward << endl;

            if (curr_dist_forward != dist_forward[forward_node]) continue;

            if (dist_backward[forward_node] < INF) {
                Dist total = dist_forward[forward_node] + dist_backward[forward_node];
                if (total < best_dist) {
                    best_dist = total;
                    best_node = forward_node;
                }
            }

            //do we not need this cjeck?
            if (curr_dist_forward >= best_dist) {
                cout << "Forward search exceeded best known distance. Stopping." << endl;
                stop_f = true;
                if (stop_b) break; // both sides done
                else continue;      // finish backward side
            }

            cout << "Relaxing Neighbors" << endl;
            //Relax upward neighbors
            for (auto& [adjacent_node, edgeInfo] : graph.up_neighbors(forward_node)) {
                Dist new_dist = dist_forward[forward_node] + (Dist)(edgeInfo.second);
                cout << "Relaxing edge: " << edgeInfo.first << " to node: " << adjacent_node << ", New Dist: " << new_dist << endl;
                if (new_dist < dist_forward[adjacent_node]) {
                    dist_forward[adjacent_node] = new_dist;
                    prev_forward[adjacent_node] = forward_node;
                    viaEdge_forward[adjacent_node] = edgeInfo.first;
                    pq_forward.push({new_dist, adjacent_node});
                    cout << "Pushed to forward PQ: " << adjacent_node << ", Dist: " << new_dist << endl;
                }
            }

        }

        else {
            //-------------------BACKWARD-------------------
            auto [curr_dist_backward, backward_node] = pq_backward.top();
            pq_backward.pop();

            cout << "Backward node: " << backward_node << ", Dist: " << curr_dist_backward << endl;

            if (curr_dist_backward != dist_backward[backward_node]) continue;

            if (dist_forward[backward_node] < INF) {
                Dist total = dist_backward[backward_node] + dist_forward[backward_node];
                if (total < best_dist) {
                    best_dist = total;
                    best_node = backward_node;
                }
            }

            // not needed?
            if (curr_dist_backward >= best_dist) {
                cout << "Backward search exceeded best known distance. Stopping." << endl;
                stop_b = true;
                if (stop_f) break; // both sides done
                else continue;      // finish forward side
            }

            //Relax upward neighbors
            for (auto& [adjacent_node, edgeInfo] : graph.up_neighbors(backward_node)) {
                Dist new_dist = dist_backward[backward_node] + (Dist)(edgeInfo.second);
                   const EdgeId edgeId_up  = edgeInfo.first;                  // v->x (up)
                    const EdgeId edgeId_rev = graph.get_edge(edgeId_up).rev_id;    // x->v (actual) <<< flip

                    cout << "Relaxing edge: " << edgeId_up << " to node: " << adjacent_node << ", New Dist: " << new_dist << endl;
                if (new_dist < dist_backward[adjacent_node]) {
                    dist_backward[adjacent_node] = new_dist;
                    // prev_backward[backward_node] = adjacent_node;
                    prev_backward[adjacent_node] = backward_node;
                    viaEdge_backward[adjacent_node] = edgeId_rev;
                    pq_backward.push({new_dist, adjacent_node});
                    cout << "Pushed to backward PQ: " << adjacent_node << ", Dist: " << new_dist << endl;
                }
            }

        }

    }

    // cout << "Backward search finished. Best node: " << best_node << ", Best dist: " << best_dist << endl;
    // cout << "ViaEdge_Forward:" << endl;
    // for (const auto& edgeId : viaEdge_forward) {
    //     cout << "Edge ID: " << edgeId << endl;
    // }

    // cout << "ViaEdge_Backward:" << endl;
    // for (const auto& edgeId : viaEdge_backward) {
    //     cout << "Edge ID: " << edgeId << endl;
    // }

    // Finished. If no path:
    if (best_dist >= INF || best_node == -1) return CH_DijkstraResult{ {}, {}, -1, {}, {} };
    return build_ch_path(prev_forward, prev_backward, viaEdge_forward, viaEdge_backward, best_node, best_dist);
}
    
vector<EdgeId> CH_Dijkstra::unpack_shortcut(Edge edge) {
    vector<EdgeId> unpacked;
    Edge edge1 = graph.get_edge(edge.sc.e_uv);
    
    Edge edge_uv = graph.get_edge(edge.sc.e_uv);
    Edge edge_vw = graph.get_edge(edge.sc.e_vw);
    // Unpack the shortcut edge into its original edges
    if (edge_uv.shortcut) {
       unpacked.insert(unpacked.end(), unpack_shortcut(edge_uv).begin(), unpack_shortcut(edge_uv).end());
    }
    if (edge_vw.shortcut) {
        unpacked.insert(unpacked.end(), unpack_shortcut(edge_vw).begin(), unpack_shortcut(edge_vw).end());
    }
    unpacked.push_back(edge_uv.id);
    unpacked.push_back(edge_vw.id);
    return unpacked;
}

CH_DijkstraResult CH_Dijkstra::build_ch_path(const vector<int>& prev_forward, const vector<int>& prev_backward, const vector<EdgeId>& viaEdge_forward, const vector<EdgeId>& viaEdge_backward, NodeId best_node, Dist best_dist) 
{
    CH_DijkstraResult result;

    result.total_cost = best_dist;

    // Reconstruct nodes from Left: src -> ... -> meet
    vector<int> left_nodes;
    vector<NodeId> ch_left_Nodes;
    vector<EdgeId> left_edges;
    vector<EdgeId> ch_left_edges;

    vector<int> right_nodes;
    vector<NodeId> ch_right_Nodes;
    vector<EdgeId> right_edges;
    vector<EdgeId> ch_right_edges;

    // vector<EdgeId> right_edges;
    // vector<EdgeId> ch_right_edges;



    for (int cur = best_node; cur != -1; cur = prev_forward[cur]) left_nodes.push_back(cur);
    reverse(left_nodes.begin(), left_nodes.end());


    for (auto& left_node: left_nodes) {
        cout << left_node << " ";
    }
    cout << endl;

    for (size_t i=1;i<left_nodes.size();++i) {
        Edge edge = graph.get_edge(viaEdge_forward[left_nodes[i]]);
        ch_left_edges.push_back(edge.id);
        if (edge.shortcut) {
            vector<EdgeId> unpacked = unpack_shortcut(edge);
            ch_left_edges.insert(ch_left_edges.end(), unpacked.begin(), unpacked.end());
        }
        left_edges.push_back(edge.id);
    }

    // for (int cur = best_node; cur != -1; cur = prev_backward[cur]) right_nodes.push_back(cur);
    // reverse(right_nodes.begin(), right_nodes.end());

    for (int cur = prev_backward[best_node]; cur != -1; cur = prev_backward[cur])
    right_nodes.push_back(cur);
    // if (right_nodes.size() > 1)
    // {
        for (size_t i=0;i<right_nodes.size();++i) {
            Edge edge = graph.get_edge(viaEdge_backward[right_nodes[i]]);
            ch_right_edges.push_back(edge.id);
            if (edge.shortcut) {
                vector<EdgeId> unpacked = unpack_shortcut(edge);
                ch_right_edges.insert(ch_right_edges.end(), unpacked.begin(), unpacked.end());
            }
            right_edges.push_back(edge.id);
        }
// }
    for (auto& left_edge: left_edges) {
        cout << left_edge << " ";
    }

    for (auto& right_edge: right_edges) {
        cout << right_edge << " ";
    }
    cout << endl;
    return result;
}

    // if at the peak there are two edegs on each side be careful

    //Reconstruct nodes from Right: meet -> ... ->
    // std::vector<int> right_nodes;
    // std::vector<EdgeId> right_edges;
    // for (int cur = best_node; cur != -1; cur = prev_backward[cur]) right_nodes.push_back(cur);
    // // reverse(right_nodes.begin(), right_nodes.end());
    // // for (int cur = prev_backward[best_node]; cur != -1; cur = prev_backward[cur])
    //     // right_nodes.push_back(cur);
    // //print right nodes
    // cout << "Right Nodes:" << endl;
    // for (const auto& node : right_nodes) {
    //     cout << "Node: " << node << endl;
    // }
    // //print viaEdge_backward
    // for (const auto& edgeId : viaEdge_backward) {
    //     cout << "viaEdge_backward ID: " << edgeId << endl;
    // }
    // // for (size_t i=0;i<right_nodes.size() - 1;++i) right_edges.push_back(viaEdge_backward[right_nodes[i]]);
    // // print right edges
    // for (int cur = best_node; prev_backward[cur] != -1; cur = prev_backward[cur])
    //     right_edges.push_back(viaEdge_backward[cur]); // meet->succ1, succ1->succ2, ...

    // cout << "Right Edges:" << endl;
    
    // result.ch_path = move(left_nodes);
    // result.ch_path.insert(result.ch_path.end(), right_nodes.begin() + 1, right_nodes.end());
    // result.edge_ids = move(left_eids);
    // result.edge_ids.insert(result.edge_ids.end(), right_edges.begin(), right_edges.end());
    // for (const auto& edgeId : result.edge_ids) {
    //     cout << "Edge ID: " << edgeId << endl;
    //     Edge edge  = graph.get_edge(edgeId);
    //     cout << "Edge from " << edge.src << " to " << edge.trg << " with cost " << edge.cost << endl;
    //     if (edge.shortcut) {
    //         cout << "Shortcut edge found!" << endl;
    //         cout << "edge 1:" << edge.sc.e_uv << endl;
    //         cout << "edge 2:" << edge.sc.e_vw << endl;
    //         cout << "middle node:" << edge.sc.middle << endl;

    //     }
    // }
    // result.total_cost = best_dist;

    // // CH path (with shortcuts) is ch.edge_ids; cost is ch.total_cost (U)
    // auto orig_eids  = unpack_path_edges(graph.get_all_edges(), result.edge_ids);
    // auto orig_nodes = edges_to_nodes(graph.get_all_edges(), orig_eids);
    // auto orig_cost  = sum_weight(graph.get_all_edges(), orig_eids);

    // //print original path information
    // cout << "Original Edge IDs:" << endl;
    // for (const auto& edgeId : orig_eids) {
    //     cout << "Edge ID: " << edgeId << endl;
    //     Edge edge  = graph.get_edge(edgeId);
    //     cout << "Edge from " << edge.src << " to " << edge.trg << " with cost " << edge.cost << endl;
    // }
    // cout << "Original Node IDs:" << endl;
    // for (const auto& nodeId : orig_nodes) {
    //     cout << "Node ID: " << nodeId << endl;
    // }
    // cout << "Original Path Cost: " << orig_cost << endl;

//     return result;
// }

// vector<NodeId> CH_Dijkstra::find_shortest_path(CH_Graph& graph, NodeId src) {
//     // Implement a bidirectional Dijkstra's algorithm on the CH graph
//     // using upward_adj and downward_adj for the search.
//     // check for the common node 
//     // Return the shortest path as a vector of NodeIds.

//     // Placeholder implementation
//     return {};
// }