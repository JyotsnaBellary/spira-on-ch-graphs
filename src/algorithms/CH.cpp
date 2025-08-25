#include "algorithms/CH.hpp"
#include <utils/utils.hpp>
#include <algorithms/dijkstra.hpp>
#include <utils/helpers.hpp>
#include <queue>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <climits>

CH::CH(Graph &graph)
  : graph(graph), 
    utils(), 
    dijkstra(graph), 
    currentEdgeDiffs(graph.num_nodes(), INT_MAX), 
    rank_order(graph.num_nodes(), INVALID_NODE),
    seen(graph.num_nodes(), 0), 
    epoch(1) {}


vector<int> CH::get_rank_order() {
    return rank_order;
}

int CH::calculate_shortcuts(NodeId nodeId)
{

    const auto &neighbors = graph.neighbors(nodeId);
    int shortcut_count = 0;

    // CLEAR existing shortcuts for this node before recomputing
    shortcutsCache[nodeId].clear();

    for (auto i = neighbors.begin(); i != neighbors.end(); ++i)
    {

        // Get the first neighbor
        const NodeId node1 = i->first;
        if (!graph.is_active(node1))
            continue;
        const Dist weight1 = static_cast<Dist>(i->second.second);

        for (auto j = std::next(i); j != neighbors.end(); ++j)
        {

            // Get the second neighbor
            const NodeId node2 = j->first;
            if (!graph.is_active(node2))
                continue;

            if (node1 == node2)
                continue; // skip self-loops

            const Dist weight2 = static_cast<Dist>(j->second.second);

            // Compute the banned capacity
            Dist bannedCap = utils.sat_add(weight1, weight2);

            // Check if a shortcut is needed
            auto [shortcutNeeded, shortcutOpType] = dijkstra.shortcut_search(node1, node2, nodeId, bannedCap);
            if (!shortcutNeeded)
                continue;

            Shortcut shortcut = {node1, node2, nodeId, bannedCap};
            shortcutsCache[nodeId].emplace_back(shortcut, shortcutOpType);
            if (shortcutOpType == ShortcutOpType::ADD)
            {
                ++shortcut_count;
            }
        }
    }
    return shortcut_count;
}

/**
 * Computes the edge difference for a given node.
 * The edge difference is defined as the number of shortcuts minus the number of active neighbors.
 */
int CH::compute_edge_difference(NodeId nodeId)
{
    // int count_active_neighbors = graph.num_nodes() - contractedNodes;
    int count_active_neighbors = graph.num_active_neighbors(nodeId);
    int shortcut_count = calculate_shortcuts(nodeId);

    // return the difference
    int edge_diff = shortcut_count - count_active_neighbors;
    return edge_diff;
}

/**
 * Recomputes the edge differences for a set of nodes.
 */
void CH::recompute_edge_differences(vector<NodeId> nodes, NodePQ& pq)
{
    // Compute the edge difference for each node
    for (NodeId node : nodes)
    {
        int edge_diff = compute_edge_difference(node);

        currentEdgeDiffs[node] = edge_diff; // Store current value
        pq.push({edge_diff, node});
    }
}

void CH::compute_rank_order()
{

    // PQ of edge differences
    NodePQ pq;
    vector<NodeId> nodesToRecompute(graph.num_nodes());

    iota(nodesToRecompute.begin(), nodesToRecompute.end(), 0);

    int shortcut_count = 0;
    int rank  = 0;
    while (contractedNodes <= graph.num_nodes())
    {
        //if there are only 1 or two nodes left, tehn contract one and continue to contract next 
        // get the new edge differences and update priority queue
        if (nodesToRecompute.size() > 0)
        {
            recompute_edge_differences(nodesToRecompute, pq);
        }

        nodesToRecompute.clear();

        // check if less than 2 active neighbors are left
        if (graph.num_nodes() - contractedNodes < 2) {
            // Contract one node and continue to contract the next
            NodeId nodeId = graph.get_any_active_node();
            graph.add_shortcuts(shortcutsCache[nodeId]);
            graph.deactivate(nodeId);
            contractedNodes++;
            rank_order[nodeId] = rank++;
            continue;
        }

        // cout << "current edge differences:" << endl;
        // for (int i =0 ; i < currentEdgeDiffs.size(); ++i) {
        //     if (graph.is_active(i)) {
        //         cout << "Node " << i << ": " << currentEdgeDiffs[i] << endl;
        //     }
        // }

        // pick a batch (start with Δ=0; bump to 1 if too small)
        auto independent_nodes = utils.independent_nodes(graph, pq, /*delta=*/1, currentEdgeDiffs);
        if (independent_nodes.empty()) {
            independent_nodes = utils.independent_nodes(graph, pq, /*delta=*/3, currentEdgeDiffs);
        }
        if (independent_nodes.empty()) break;  // nothing valid left

        // epoch-based dedup for next recompute set
        auto push_unique = [&](NodeId v) {
            if (!graph.is_active(v)) return;
            if (seen[v] != epoch) { seen[v] = epoch; nodesToRecompute.push_back(v); }
        };

        std::vector<std::vector<NodeId>> nbrs_of(independent_nodes.size());
        // cout << "following nodes are part of independent set:" << endl;
        for (size_t i = 0; i < independent_nodes.size(); ++i) {
            NodeId nodeId = independent_nodes[i];
            if (!graph.is_active(nodeId)) continue; // skip if already contracted
            // cout << "Node " << nodeId << endl;
            shortcut_count += graph.add_shortcuts(shortcutsCache[nodeId]);
            graph.deactivate(nodeId);
            // cout << "Node " << nodeId << " deactivated." << endl;
            // number of active nodes left

            //maybe write an independent func for it?
            for (const auto& [v, e] : graph.neighbors(nodeId)) {
                if (graph.is_active(v)) nbrs_of[i].push_back(v);
            }

             // schedule recompute for its (previously snapshotted) neighbors
            for (NodeId v : nbrs_of[i]) push_unique(v);
            rank_order[nodeId] = rank++;
        }

        ++epoch;  // new round for deduper
        contractedNodes += (int)independent_nodes.size();


    }
    // find independent set of nodes until least to the median edge difference value
    // contract them
    // mark the node as inactive.
    //
    // recalculate edge differences for neighbours of contracted nodes
    cout << "number of shortcuts added: " << shortcut_count << endl;
}

void CH::preprocess()
{
    // Preprocessing implementation for Contraction Hierarchies
    // This function should prepare the graph for efficient queries

    // compute rank order
    compute_rank_order();
    // create and store upwards and downwards graphs
    // cout << "rank order :" << endl;
    // for (int i = 0; i < rank_order.size(); ++i) {
    //     cout << "Node " << i << ": " << rank_order[i] << endl;
    // }
    return;
}