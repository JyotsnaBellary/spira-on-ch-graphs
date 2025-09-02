#include <algorithms/CCH.hpp>
#include <io/file_handler.hpp>
#include <utils/kahip_runner.hpp>

using namespace std::chrono;

CCH::CCH(Graph &graph) : graph(graph) {}

// void CCH::customization() {

// }

void CCH::preprocess()
{

    cout << "number of nodes:" << graph.num_nodes() << endl;
    cout << "number of edges:" << graph.num_edges() << endl; 
    auto start_ch = high_resolution_clock::now();
    vector<int> contraction_order = compute_contraction_order();
    // cout << "contraction order" << endl;
    // for (int i = 0; i < contraction_order.size(); i++) {
    //     cout << "rank " << i << ": " << contraction_order[i] << endl;;
    // }

    // cout << "rank array" << endl;
    // for (int i = 0; i < rank_of_node.size(); i++) {
    //     cout << "node_ranks[" <<  i << "]: " << rank_of_node[i] << endl;

    // }
    auto end_ch = high_resolution_clock::now();

          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_ch - start_ch);

          auto h = std::chrono::duration_cast<std::chrono::hours>(duration);
          duration -= h;

          auto m = std::chrono::duration_cast<std::chrono::minutes>(duration);
          duration -= m;

          auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
          duration -= s;

          auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

          std::cout << "Node Ordering time: "
                    << h.count() << " h "
                    << m.count() << " m "
                    << s.count() << " s "
                    << ms.count() << " ms" << std::endl;

    auto start_lower = high_resolution_clock::now();

    compute_lower_triangles(contraction_order);
    set_shortcut_rank();
    auto end_lower = high_resolution_clock::now();

     duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_lower - start_lower);

           h = std::chrono::duration_cast<std::chrono::hours>(duration);
          duration -= h;

           m = std::chrono::duration_cast<std::chrono::minutes>(duration);
          duration -= m;

           s = std::chrono::duration_cast<std::chrono::seconds>(duration);
          duration -= s;

           ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

        //   std::cout << "Contraction order preprocessing time: "
        //             << h.count() << " h "
        //             << m.count() << " m "
        //             << s.count() << " s "
        //             << ms.count() << " ms" << std::endl;

    //write a test for this

    // print lower triangles
    std::cout << "Lower Triangles processing time: "
              << h.count() << " h " << m.count() << " m "
              << s.count() << " s " << ms.count() << " ms\n";
    // cout << "finished shorting" << endl;
    // print_shortcuts_by_trg_order();
    

    // Debug (guard or sample on large instances)
    // if (shortcutsCache.size() <= 100) {
        // for (size_t i = 0; i < shortcutsCache.size(); ++i) {
        //     const auto& e = shortcutsCache[i];
        //     if (e.shortcut) {
        //         std::cout << "shortcut between " << e.src << " and " << e.trg << "\n  lower triangles: [";
        //         for (auto w : lower_triangle_nodes[i]) std::cout << w << ", ";
        //         // std::cout << "]\n  ranks: [";
        //         // for (auto w : lower_triangle_nodes[i]) std::cout << node_ranks[w] << ", ";
        //         std::cout << "]\n";
        //     }
            
        // }
    // }
    std::cout << "Number of triangles to customize: " << shortcutsCache.size() << "\n";
    return;

}

void CCH::print_shortcuts_by_trg_order() const {
for (int r = 0;  r< shortcut_of_rank_by_trg.size(); r++) {
 size_t idx = shortcut_of_rank_by_trg[r];
        const Edge& e = shortcutsCache[idx];
        std::cout << "rank " << r
                  << " : shortcut (" << e.src << " -> " << e.trg << ")  v=" << e.trg
                  << " | middles: [";
    

    const auto& mids = lower_triangle_nodes[idx]; // print in stored order
        for (size_t i = 0; i < mids.size(); ++i) {
            if (i) std::cout << ", ";
            NodeId w = mids[i];
            std::cout << w << "(r=" << rank_of_node[w] << ")";
        }
    std::cout << "]\n";
}
}
void CCH::set_shortcut_rank()
{
    const size_t M = shortcutsCache.size();

    // 1) build cached signatures of middle-node ranks (DESC)
    mid_rank_sig_desc.assign(M, {});
    for (size_t i = 0; i < M; ++i) {
        const auto& mids = lower_triangle_nodes[i];
        auto& sig = mid_rank_sig_desc[i];
        sig.reserve(mids.size());
        for (NodeId w : mids) sig.push_back(rank_of_node[w]);
        std::sort(sig.begin(), sig.end(), std::greater<int>());
    }

    // 2) sort an index by (v asc), then lexicographic sig desc
    shortcut_of_rank_by_trg.resize(M);
    std::iota(shortcut_of_rank_by_trg.begin(), shortcut_of_rank_by_trg.end(), 0);

    auto lex_desc = [&](const std::vector<int>& A, const std::vector<int>& B) {
        const size_t L = std::min(A.size(), B.size());
        for (size_t i = 0; i < L; ++i) {
            if (A[i] != B[i]) return A[i] < B[i]; // lower rank wins
        }
        // identical prefix: longer list wins (has a next comparison term)
        // if (A.size() != B.size()) return A.size() > B.size();
        return false; // equal
    };

    std::stable_sort(shortcut_of_rank_by_trg.begin(), shortcut_of_rank_by_trg.end(),
        [&](size_t ia, size_t ib) {
            const Edge& ea = shortcutsCache[ia];
            const Edge& eb = shortcutsCache[ib];
            // NodeId va = effective_trg(ea);
            // NodeId vb = effective_trg(eb);
            #ifndef NDEBUG
            assert_upward(ea);
            assert_upward(eb);
            #endif
            if (ea.trg != eb.trg) return ea.trg < eb.trg;          // primary: v ascending
            if (lex_desc(mid_rank_sig_desc[ia], mid_rank_sig_desc[ib])) return true;
            if (lex_desc(mid_rank_sig_desc[ib], mid_rank_sig_desc[ia])) return false;
            // final tiebreakers for deterministic order:
            if (ea.src != eb.src) return ea.src < eb.src; //which have lower src
            return ia < ib;
        });

    // 3) build inverse mapping
    rank_of_shortcut_by_trg.assign(M, -1);
    for (size_t r = 0; r < M; ++r)
        rank_of_shortcut_by_trg[shortcut_of_rank_by_trg[r]] = static_cast<int>(r);
}

int CCH::add_shortcuts(const vector<NodeId> &neighbors, NodeId middle_node)
{
    const int k = neighbors.size();
if (k < 2) {
        return 0;
    }
    // Fewer reallocs when degrees are large
    shortcutsCache.reserve(shortcutsCache.size() + k*(k-1)/2);
    lower_triangle_nodes.reserve(lower_triangle_nodes.size() + k*(k-1)/2);

    int new_edges = 0;
    for (auto i = 0; i < k; i++)
    {
        NodeId node1 = neighbors[i];
        // if (!graph.is_active(node1))
        //     continue;
        for (auto j = i + 1; j < k; j++)
        {
            NodeId node2 = neighbors[j];
            // if (!graph.is_active(node2))
            //     continue;

            if (node1 == node2)
                continue;

                // decide direction by rank so v is always “higher”
            if (rank_of_node[node1] > rank_of_node[node2]) swap(node1, node2);


            uint64_t key = pair_key(node1, node2);
            auto it = shortcutPos.find(key);

            // auto it1 = shortcutPositionLookup.find(node1);
            if (it != shortcutPos.end())
            {
                lower_triangle_nodes[it->second].push_back(middle_node);
                continue;
            }
            // Edge edge;
            if (!graph.edge_exists(node1, node2))
            {
                
                Edge edge = {graph.num_edges(), graph.num_edges() + 1, node1, node2, INF_WEIGHT, true, true};
                Edge rev_edge = {edge.rev_id, edge.id, node2, node1, INF_WEIGHT, true, true};
                graph.set_edge(edge, rev_edge);
                // graph.edges.push_back(edge);
                // graph.edges.push_back(rev_edge);
                new_edges += 1;
                // cout << "Added shortcut between " << edge.src << "->" << edge.trg << endl;
                // edge = graph.get_edge_by_src_dst(node1, node2);

                // if shortcut/org edge exists, let its weight stay but add it to the cache list along with the lower mode ranks
            }
            // else
            // {
                
            // }

             
                // create new shortcut record
                //should not be doing this. should not be considering your new neighbors. find another wa to store your shortcuts
                const Edge& e = graph.get_edge(node1, node2); // O(1)
                shortcutsCache.emplace_back(e);
                size_t pos = shortcutsCache.size() - 1;
                shortcutPos.emplace(key, pos);
                // start triangle list with one element efficiently
                lower_triangle_nodes.emplace_back(1, middle_node);
            
        //         if (it2 != it1->second.end())
        //         {
        //             //  Exists
        //             size_t position = it2->second;
        //             // if it exists in shortcache
        //             //  check using shortcutpositionlookup if an edge here exists
        //             //  size_t position = shortcutPositionLookup[node1][node2];

        //             // now if this position exists, add middle node to the vector of middle nodes in array of middle nodes
        //             lower_triangle_nodes[position].push_back(middle_node);
        //         }
        //         else {
        //             // MISSING CASE — add new shortcut
        //             shortcutsCache.emplace_back(edge);
        //             size_t pos = shortcutsCache.size() - 1;
        //             inner[node2] = pos;
        //             lower_triangle_nodes.emplace_back(1, middle_node);
        //         }
        //     }
        //     else
        //     {
        //         // if does not exist, then
        //         shortcutsCache.emplace_back(edge);
        //         int position = shortcutsCache.size() - 1;
        //         // store position in shortcutsCache
        //         shortcutPositionLookup[node1][node2] = position;
        //         vector<NodeId> lower_nodes;
        //         lower_nodes.push_back(middle_node);
        //         lower_triangle_nodes.emplace_back(lower_nodes);

        //         // get teh edge id. add it to the shortcut cache
        //         //  now in shortcut cache , let its position be the shortcut cache id.
        //         // maintain a vector of vector of lower nodes that form lower triangle with each of these shortcuts
        //     }
        }
    }

    
    return new_edges;
}

void CCH::compute_lower_triangles(const vector<NodeId> &contraction_order)
{
    int new_edges = 0;
    for (int i = 0; i < contraction_order.size(); i++)
    {
        NodeId nodeId = contraction_order[i];
        vector<NodeId> neighbors = graph.get_sorted_higher_original_neighbors(nodeId, rank_of_node);
        new_edges += add_shortcuts(neighbors, nodeId);
        graph.deactivate(nodeId);
    }
    cout << "Shortcuts Added: " << new_edges << endl;
}

vector<NodeId> CCH::compute_contraction_order()
{
    // Implementation of rank order computation

    vector<int> reverse_contracted_nodes;
    // vector<int> nodes_to_dissect;
    // compute_nested_dissection(graph.get_all_node_ids(), reverse_contracted_nodes );

    FileHandler fh;

    fh.write_to_metis_format(graph, "./RoadNetworks/test.graph");

    string output_file_path = "";
    try
    {
        output_file_path = run_kahip_on_test_graph("level2");
        // std::cout << "KaHIP wrote: " << out << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return {};
    }

    Ordering ordering = fh.read_kahip_output(output_file_path, graph.num_nodes());
    vector<NodeId> contraction_order = ordering.node_of_rank;
    // contraction_order = {0,1,2,3,4,5,6,7}; //assignment
    // contraction_order = {2,0,4,7,5,3,1,6}; //my test

    rank_of_node = ordering.rank_of_node;
    // rank_of_node = {1,2,3,4,5,6,7,8}; assignment
    // rank_of_node = {2, 7, 1, 6, 3, 5, 8, 4}; //my test

    // cout << "contraction order is" << endl;
    // for (auto &node : contraction_order)
    // {
    //     cout << node << endl;
    // }

    // cout << "ranks" << endl;
    // for (int i = 0; i < node_ranks.size(); i++)
    // {
    //     cout << "rank of node " << i << ":" << node_ranks[i] << endl;
    // }
    // if (output.left.empty() && output.right.empty() && output.separator.empty()) {
    //     cerr << "Error: No partitions found in KaHIP output.\n";
    //     return;
    // }
    return contraction_order;
}

// void CCH::compute_nested_dissection(vector<int> nodes, vector<NodeId>& reverse_contracted_nodes) {
//     // Implementation of nested dissection
//     // call file handler to prepare a .file graph
//     // in this file handler, open a file and write the following
//     // n m
//     // start from node id = 2 as line id is 2
//     // (for node id 1, add it to other edge end)
//     // as you mark an edge, mark that its reverse has been added as well.
//     // since my nodes start with 0, I need to do +1 for all before adding it to file.
//     // check for inactive nodes, only if they are active.
//     // call the node separator on this graph file

//     //first round, all nodes will be assigned. start with nodes id 0. it will be mapped to 1 and then 2 and so on

//     //maintain a vector starting with index 1. let index 0 be -1 (never used )
//     vector<int> lineId_to_nodeId;
//     unordered_map<int,int> nodeId_to_lineId;
//     // nodeId_to_lineId.reserve(nodes.size());
//     lineId_to_nodeId.push_back(-1); //index 0 is never used.

//     for (auto i = 0; i < nodes.size(); i++) {
//         lineId_to_nodeId.push_back(nodes[i]);
//         nodeId_to_lineId[nodes[i]] = i + 1; // Map global node ID to local index
//     }

//     FileHandler fh;

//     // fh.write_to_metis_format(lineId_to_nodeId, nodeId_to_lineId, graph, "./RoadNetworks/test.graph");
//     string output_file_path = "";
//     try {
//         output_file_path = run_kahip_on_test_graph("level2");
//     // std::cout << "KaHIP wrote: " << out << "\n";
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << "\n";
//         return;
//     }

//     Kahip_Output output = fh.read_kahip_output(output_file_path, nodes.size());
//     if (output.left.empty() && output.right.empty() && output.separator.empty()) {
//         cerr << "Error: No partitions found in KaHIP output.\n";
//         return;
//     }

//     // vector<int> left_nodes;
//     // vector<int> right_nodes;

//     // //print separator output
//     // // for (int i = 0; i < output.separator.size(); i++) {
//     // //     cout << output.separator[i];
//     // // }
//     // // cout << endl;

//     // if (output.separator.size() > 0) {
//     //     for (const auto& node: output.separator) {
//     //         NodeId node_id = lineId_to_nodeId[node];
//     //         graph.deactivate(node_id);
//     //         reverse_contracted_nodes.push_back(node_id);
//     //     }
//     // }

//     // if (output.left.size() <=2 ) {
//     //     for (const auto& lineId: output.left) {
//     //         reverse_contracted_nodes.push_back(lineId_to_nodeId[lineId]);
//     //         graph.deactivate(nodeId_to_lineId[lineId]);
//     //     }

//     //     //do I return here?. no check if right
//     // }
//     // //what if there are two nodes
//     // // if (output.left.size() == 2) {
//     // //     reverse_contracted_nodes.push_back(lineId_to_nodeId[output.left[0]]);
//     // //     graph.deactivate(nodeId_to_lineId[output.left[0]]);
//     // //     //do I return here?. no check if right
//     // // }
//     // else if (output.left.size() > 2) {
//     //     // push
//     //     for (auto lineId: output.left) {
//     //         //if it has no neighbors, it can be pushed to reverse_contracted_nodes
//     //         if (graph.num_active_neighbors(lineId_to_nodeId[lineId]) > 0) {
//     //             left_nodes.push_back(lineId_to_nodeId[lineId]);
//     //             continue;
//     //         }
//     //         reverse_contracted_nodes.push_back(lineId_to_nodeId[lineId]);
//     //         graph.deactivate(nodeId_to_lineId[output.left[lineId]]);
//     //     }
//     // }

//     // if (output.right.size() <=2 ) {
//     //     for (const auto& lineId: output.right) {
//     //         reverse_contracted_nodes.push_back(lineId_to_nodeId[lineId]);
//     //         graph.deactivate(nodeId_to_lineId[lineId]);
//     //     }
//     // }
//     // else if (output.right.size() > 2) {
//     //     //if it has no neighbors, it can be pushed to reverse_contracted_nodes
//     //     for (auto lineId: output.right) {
//     //         if (graph.num_active_neighbors(lineId_to_nodeId[lineId]) > 0) {
//     //             right_nodes.push_back(lineId_to_nodeId[lineId]);
//     //             continue;
//     //         }
//     //         reverse_contracted_nodes.push_back(lineId_to_nodeId[lineId]);
//     //         graph.deactivate(nodeId_to_lineId[output.left[lineId]]);
//     //     }
//     // }

//     // if (left_nodes.size() > 1) {
//     //     compute_nested_dissection(left_nodes, reverse_contracted_nodes);
//     // }

//     // if (right_nodes.size() > 1) {
//     //     compute_nested_dissection(right_nodes, reverse_contracted_nodes);
//     // }

//     return;

//     // get a seperator
//     // write each parts to seperate files and keep calling again and again
//     // until we reach a base case i.e. we get individual nodes

//     // these individual nodes will be given higher rank priority
//     // then the ones at the upper level and teh upper than that and so on.

//     // at each level, the nodes are basically ranked in any random order but they are all from same level.

//     // need to write a function to

// }





// void CCH::remove_shortcut(NodeId u, NodeId v) {
//     // Implementation of removing a shortcut
// }

// const vector<pair<Shortcut, ShortcutOpType>>& CCH::get_shortcuts(NodeId nodeId) const {
//     // Implementation of getting shortcuts for a node
//     return shortcutsCache;
// }
