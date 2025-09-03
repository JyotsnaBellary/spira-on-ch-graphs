#include "algorithms/CH.hpp"
#include <utils/utils.hpp>
#include <utils/helpers.hpp>
#include <algorithms/witness_dijkstra.hpp>
#include <queue>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <climits>

CH::CH(Graph &graph)
    : graph(graph),
      utils(),
      witnessDijkstra(graph),
      currentEdgeDiffs(graph.num_nodes(), INT_MAX),
      rank_order(graph.num_nodes(), INVALID_NODE),
      seen(graph.num_nodes(), 0),
      epoch(1) {}

vector<int> CH::get_rank_order()
{
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
            auto [shortcutNeeded, shortcutOpType] = witnessDijkstra.shortcut_search(node1, node2, nodeId, bannedCap);
            if (!shortcutNeeded)
                continue;

            Shortcut shortcut = {node1, node2, nodeId, bannedCap};
            //adding shortcut for nodeId. print

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
    int count_active_neighbors = graph.num_active_neighbors(nodeId);
    if (count_active_neighbors < 2)
    { // no pairs, no witness runs
        shortcutsCache[nodeId].clear();
        return 0 - count_active_neighbors; // edge diff = adds(0) - deg
    }
    int shortcut_count = calculate_shortcuts(nodeId);
    // return the difference
    int edge_diff = shortcut_count - count_active_neighbors;
    return edge_diff;
}

/**
 * Recomputes the edge differences for a set of nodes.
 */
void CH::recompute_edge_differences(vector<NodeId> nodes, NodePQ &pq)
{
    // Compute the edge difference for each node
    for (NodeId node : nodes)
    {
        int edge_diff = compute_edge_difference(node);

        currentEdgeDiffs[node] = edge_diff; // Store current value
        pq.push({edge_diff, node});
    }
}

int CH::compute_rank_order()
{

    // PQ of edge differences
    NodePQ pq;
    static std::vector<int> median_scratch;
    vector<NodeId> nodesToRecompute(graph.num_nodes());
    iota(nodesToRecompute.begin(), nodesToRecompute.end(), 0);

    int shortcut_count = 0;
    int rank = 0;

    // helper: seed a few unknown scores so PQ has candidates (no full recompute)
    auto seed_some_unknowns = [&](int limit = 1000)
    {
        int seeded = 0;
        for (NodeId u = 0; u < graph.num_nodes() && seeded < limit; ++u)
        {
            if (!graph.is_active(u))
                continue;
            if (currentEdgeDiffs[u] == INT_MAX)
            {
                int deg = graph.num_active_neighbors(u);
                int adds = (deg < 2) ? 0 : calculate_shortcuts(u);
                currentEdgeDiffs[u] = adds - deg;
                pq.push({currentEdgeDiffs[u], u});
                ++seeded;
            }
        }
        return seeded;
    };


    // helper: pick best valid from PQ (skip stale/inactive)
    auto pick_best_from_pq = [&](NodePQ &pq) -> NodeId
    {
        while (!pq.empty())
        {
            auto [k, u] = pq.top();
            pq.pop();
            if (!graph.is_active(u))
                continue;
            if (k != currentEdgeDiffs[u])
                continue; // stale
            return u;
        }
        return -1;
    };

    while (contractedNodes < graph.num_nodes())
    {
        // if there are only 1 or two nodes left, tehn contract one and continue to contract next
        //  get the new edge differences and update priority queue
        if (nodesToRecompute.size() > 0)
        {
            recompute_edge_differences(nodesToRecompute, pq);
            nodesToRecompute.clear();
        }

        // check if less than 2 active neighbors are left
        if (graph.num_nodes() - contractedNodes < 2)
        {
            if (graph.num_nodes() - contractedNodes == 0)
                break;
            // Contract one node and continue to contract the next
            NodeId nodeId = graph.get_any_active_node();
            if (nodeId == -1)
                break; // no active nodes left
            shortcut_count += graph.add_shortcuts(shortcutsCache[nodeId]);
            graph.deactivate(nodeId);

            contractedNodes++;
            rank_order[nodeId] = rank++;
            ++epoch;
            continue;
        }

        size_t remaining = graph.num_nodes() - contractedNodes;
        size_t target = std::clamp(remaining / 500, size_t(64), size_t(5000)); // ~0.2% of remaining

        int median = utils.compute_active_median(graph, currentEdgeDiffs, median_scratch);
        if (median == INT_MAX)
        {
            if (seed_some_unknowns(1000) == 0)
                break; // truly done
            continue;
        }

        auto independent_nodes = utils.independent_nodes(graph, pq, median, currentEdgeDiffs);
        if (independent_nodes.empty())
        {
            independent_nodes = utils.independent_nodes(graph, pq, /*delta=*/median +1, currentEdgeDiffs);
        }
        if (independent_nodes.empty())
        {
            // added new
            //  fallback: contract one best node to guarantee progress
            NodeId u = pick_best_from_pq(pq);
            if (u != -1)
                independent_nodes.push_back(u);
            else
            {
                // if heap starved but actives remain, seed a few and continue
                if (seed_some_unknowns(1000) == 0)
                    break;
                continue;
            }
        }; // nothing valid left

        // epoch-based dedup for next recompute set
        auto push_unique = [&](NodeId v)
        {
            if (!graph.is_active(v))
                return;
            if (seen[v] != epoch)
            {
                seen[v] = epoch;
                nodesToRecompute.push_back(v);
            }
        };

        //  snapshot neighbors for the whole batch BEFORE mutating
        std::vector<std::vector<NodeId>> nbrs_of(independent_nodes.size());
        for (size_t i = 0; i < independent_nodes.size(); ++i)
        {
            NodeId u = independent_nodes[i];
            if (!graph.is_active(u))
                continue;
            for (auto &&[v, e] : graph.neighbors(u))
                if (graph.is_active(v))
                    nbrs_of[i].push_back(v);
        }

        int deactivated = 0;
        for (size_t i = 0; i < independent_nodes.size(); ++i)
        {
            NodeId nodeId = independent_nodes[i];
            if (!graph.is_active(nodeId))
                continue; // skip if already contracted

            shortcut_count += graph.add_shortcuts(shortcutsCache[nodeId]);
            graph.deactivate(nodeId);
            deactivated++;
            rank_order[nodeId] = rank++;
        }
        ++epoch; // new round for deduper
        contractedNodes += deactivated;
        
        for (auto &snap : nbrs_of)
            for (NodeId v : snap)
                push_unique(v);
    }

    // recalculate edge differences for neighbours of contracted nodes
    // cout << "number of shortcuts added: " << shortcut_count << endl;

    // Check if ranks are INT_MAX
    int errcount = 0;
    for (NodeId nodeId = 0; nodeId < graph.num_nodes(); ++nodeId)
    {
        if (rank_order[nodeId] == INT_MAX)
        {
            errcount++;
        }
    }

    // check if same rank has been assigned to multiple nodes
    std::unordered_map<int, std::vector<NodeId>> rank_groups;
    for (NodeId nodeId = 0; nodeId < graph.num_nodes(); ++nodeId)
    {
        if (rank_order[nodeId] != INT_MAX)
        {
            rank_groups[rank_order[nodeId]].push_back(nodeId);
        }
    }
    for (const auto &[rank, nodes] : rank_groups)
    {
        if (nodes.size() > 1)
        {
            cerr << "Warning: Rank " << rank << " assigned to multiple nodes: ";
            for (NodeId nodeId : nodes)
            {
                cerr << nodeId << " ";
            }
            cerr << endl;
        }
    }

    cerr << "Number of nodes with INT_MAX rank: " << errcount << endl;
    return shortcut_count;
}

int CH::preprocess()
{
    int shortcuts = compute_rank_order();
    return shortcuts;
}