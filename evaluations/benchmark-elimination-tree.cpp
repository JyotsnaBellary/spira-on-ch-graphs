#include <benchmark-elimination-tree.hpp>

using namespace std;
using Clock = std::chrono::steady_clock;

void BenchmarkEliminationTree::run_dijkstra_with_elimination_tree(NodeId src, NodeId dst)
{
    cout << "running CCH with elimination tree on OSM 1" << endl;

    string filepath = "./Input_Data/SparseRoadNetworks/osm1.txt";
    // int src, dst;

    // src = 1;
    // dst = 250;
    cerr << "Running Dijkstra on " << filepath
         << " src=" << src << " dst=" << dst << endl;

    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file(filepath);

    CCH cch(graph);
    cch.compute_contraction_order();
    cout << "Computed Seperator Decomposition. Processing Graph now!" << endl;

    CCH_Result cch_res = cch.preprocess();
    cout << "Preprocessed graph. Shortcuts added: " << cch_res.shortcuts << endl;
    cout << "adding customized weights" << endl;
    // Assign weight of shortcuts
    cch.customization();

    cout << "customization done" << endl;

    // ch.build_upward_adj();
    // ch.build_downward_adj();

    // Now run spira quesries on the upward and downward adjacency lists
    //  CH_Graph chGraph(graph.get_all_nodes(), graph.get_all_edges(), ch.get_rank_order(), true);

    // here we have the upward adj list ready but not in and out list. we dont ned it now
    CH_Graph cch_graph(cch.get_graph().get_all_nodes(), cch.get_graph().get_all_edges(), cch.get_ranks(), true);

    cch_graph.shortcuts = cch_res.shortcuts;
    cout << "CCH graph ready." << endl;

    // PRUNE it more
    // now for the source destination, I need reachable query graphs
    Query_Graph_Type reachable_query_graph = cch_graph.get_upward_graph(src, dst);

    // maybe the input is wrong. need to merge the two reachable graphs to get to the final query graph.
    CH_Graph query_graph(reachable_query_graph.reachable_nodes, reachable_query_graph.all_edges, reachable_query_graph.rank, false);
    query_graph.shortcuts = cch_graph.shortcuts;

    query_graph.set_upward_adj_lists(reachable_query_graph.out_adjacency_list, reachable_query_graph.in_adjacency_list);

    // query_graph.print_adj_in();
    query_graph.print_adj_out();
    cout << query_graph.number_of_nodes() << " nodes and" << query_graph.number_of_edges() << " edges in the query graph.\n";

    // print the src and dst in the new graph
    NodeId new_src = reachable_query_graph.old_to_new[(int)src];
    NodeId new_dst = reachable_query_graph.old_to_new[(int)dst];
    // build elimination tree for the query graph
    EliminationTree eliminationTree(query_graph);

    // print old to new mapping ffor only the reachable nodes
    cout << "Old to new mapping for reachable nodes: " << endl;
    for (NodeId old_id : reachable_query_graph.old_node_ids)
    {
        cout << "Old ID: " << old_id
             << " -> New ID: " << reachable_query_graph.old_to_new[old_id]
             << endl;
    }
    eliminationTree.print_elimination_tree();

        Bi_Directional_Dijkstra dijkstra(query_graph);
    auto start_pq = Clock::now();
    Bi_Directional_Dijkstra_Result dijkstra_result =
        dijkstra.compute_shortest_path(new_src, new_dst);
    auto end_pq = Clock::now();

    double pq_ms =
        std::chrono::duration<double, std::milli>(end_pq - start_pq).count();

    // print the path and the cost
    cout << "Dijkstra complete. Shortest path cost: "
         << dijkstra_result.total_cost << endl;
    cout << "Path: ";
    for (NodeId node : dijkstra_result.path)
    {
        cout << node << " ";
    }
    cout << endl;
    cout << "Number of PQ pops: " << dijkstra_result.number_of_pops << endl;
    cout << "Runtime: " << pq_ms << " ms" << endl;

    // auto start_et = Clock::now();
    // Bi_Directional_Dijkstra_Result dijkstra_result_et =
    //     dijkstra.compute_shortest_path(new_src, new_dst, SearchMode::EliminationTree);
    // auto end_et = Clock::now();

    // double et_ms =
    //     std::chrono::duration<double, std::milli>(end_et - start_et).count();

    // cout << "Dijkstra with elimination tree complete. Shortest path cost: "
    //      << dijkstra_result_et.total_cost << endl;
    // cout << "Path: ";
    // for (NodeId node : dijkstra_result_et.path)
    // {
    //     cout << node << " ";
    // }
    // cout << endl;
    // cout << "Number of ET pops: " << dijkstra_result_et.number_of_pops << endl;
    // cout << "Runtime: " << et_ms << " ms" << endl;

    // Bi_Directional_Spira spira(query_graph);
    // Bi_Directional_Dijkstra_Result spira_result_et = spira.compute_shortest_path(new_src, new_dst);

    // cout << "bi directional Spira on original graph complete. Shortest path cost: " << spira_result_et.total_cost << endl;
    // cout << "Path: ";
    // for (NodeId node : spira_result_et.path)
    // {
    //     cout << node << " ";
    // }
    // cout << endl;
    // cout << "Number of PQ pops: " << spira_result_et.number_of_pops << endl;

    // cout << "Dijkstra complete. Shortest path cost: " << dijkstra_result.total_cost << endl;
    // // in SPT mode, print distances to nodes that are reachable from src
    // for (size_t i = 0; i < dijkstra_result.distance.size(); ++i) {
    //     if (dijkstra_result.distance[i] < INF_COST) {
    //         cout << "Distance from " << src << " to " << i << ": " << dijkstra_result.distance[i] << endl;
    //     }
    // }
    // cout << "Number of PQ pops: " << dijkstra_result.number_of_pops << endl;

    // // chGraph.print_adj_out();
    // chGraph.sort_all_neighbors();
    // Spira spira(chGraph);
    //  // Optional: print the rank order of nodes
    // SsspResult spira_result = spira.compute_shortest_path(src, dst);

    // cout << "Spira complete. Shortest path cost: " << spira_result.total_cost << endl;

    // // in SPT mode, print distances to nodes that are reachable from src
    // for (size_t i = 0; i < spira_result.distance.size(); ++i) {
    //     if (spira_result.distance[i] < INF_COST) {
    //         cout << "Distance from " << src << " to " << i << ": " << spira_result.distance[i] << endl;
    //     }
    // }

    // cout << "Number of PQ pops: " << spira_result.number_of_pops << endl;

    // NewVariant newVariant(chGraph);
    // SsspResult new_variant_result = newVariant.compute_shortest_path(src, dst);

    // cout << "New Variant complete. Shortest path cost: " << new_variant_result.total_cost << endl;

    // // in SPT mode, print distances to nodes that are reachable from src
    // for (size_t i = 0; i < new_variant_result.distance.size(); ++i) {
    //     if (new_variant_result.distance[i] < INF_COST) {
    //         cout << "Distance from " << src << " to " << i << ": " << new_variant_result.distance[i] << endl;
    //     }
    // }

    // cout << "Number of PQ pops: " << new_variant_result.number_of_pops << endl;

    return;
}

// for single run
void BenchmarkEliminationTree::run_dijkstra_with_priority_queue() {}

// for 100 runs
void BenchmarkEliminationTree::run_src_dst_benchmark_with_elimination_tree(const CH_Graph &graph, const string &output_csv_path) {}
