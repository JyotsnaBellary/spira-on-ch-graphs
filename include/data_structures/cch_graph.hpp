#pragma once
#include <unordered_map>
#include <vector>
#include "data_structures/node.hpp"
#include "data_structures/edge.hpp"
#include <stdexcept>
#include <iostream>
#include <optional>

using namespace std;

struct CCH_Graph
{
    vector<Node> nodes; // size n, nodes[i].id == i
    vector<Edge> edges; // flat edge storage            
    vector<uint8_t> active;
            // 1=active, 0=inactive
    std::unordered_map<uint64_t, EdgeId> edge_index;
    // do I need tgis?
    // vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_edges_adj;
    // vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_edges_adj;
    vector<int> rank;

    // struct ShortcutRef
    // {
    //     EdgeId e_vu, e_vw;
    //     NodeId m;
    // };
    // std::vector<std::vector<ShortcutRef>> tri_buckets; // per edge
    // std::unordered_map<EdgeId, std::vector<ShortcutRef>> tri_buckets;
    std::vector<std::vector<EdgeId>> buckets_by_rank;

    CCH_Graph() = default;
    inline const vector<Edge> &get_all_edges() const { return edges; }
    inline void deactivate(NodeId v) { active[v] = 0; }

    // resize nodes, edges, active and adj
    CCH_Graph(vector<Node> all_nodes, vector<Edge> all_edges, vector<NodeId> rank_order)
    {
        nodes = all_nodes;
        edges = all_edges;
        cout << "Initialized with " << edges.size() << " edges." << endl;
        upward_edges_adj.resize(nodes.size());
        rank = rank_order;
        active.resize(nodes.size(), 1);

        // build upward and downward adj list
        build_upward_adj_Lists();
    }

    inline bool is_up(NodeId u, NodeId v) { return rank[v] > rank[u]; } // your convention

    int num_nodes() const { return static_cast<int>(nodes.size()); }
    // inline const Edge &get_edge(EdgeId id) const { return edges[id]; }

    const unordered_map<NodeId, pair<EdgeId, Weight>> &up_neighbors(NodeId node) const
    {
        if (node < 0 || node >= static_cast<int>(upward_edges_adj.size()))
        {
            throw out_of_range("Invalid node ID");
        }
        return upward_edges_adj[node];
    }

    // Pack (u,v) into a 64-bit key. Requires rank[u] < rank[v].
    inline uint64_t edge_key(NodeId u, NodeId v) const
    {
        return (uint64_t(uint32_t(u)) << 32) | uint64_t(uint32_t(v));
    }

    vector<NodeId> get_sorted_higher_neighbors(NodeId nodeId, vector<int> &node_rank) const
    {

        vector<NodeId> up_neighbors;
        up_neighbors.reserve(upward_edges_adj[nodeId].size());

        // Iterate the adjacency map directly; neighbors(v) is fine too.
        for (const auto &[neighbor_id, _] : upward_edges_adj[nodeId])
        {
            // Guard against corrupted keys
            if (neighbor_id < 0 || static_cast<size_t>(neighbor_id) >= node_rank.size())
                continue;
            if (node_rank[neighbor_id] > node_rank[nodeId])
                if (active[neighbor_id])
                    up_neighbors.push_back(neighbor_id);
        }

        sort(up_neighbors.begin(), up_neighbors.end(),
             [&](NodeId a, NodeId b)
             { return node_rank[a] < node_rank[b]; });

        return up_neighbors;
    }

    // Create a brand-new upward edge (assumes it does not exist yet).
    EdgeId add_upward_edge(NodeId u, NodeId v, Weight w, bool shortcut, EdgeId edgeId = INVALID_EDGE)
    {
        EdgeId id;

        if (edgeId == INVALID_EDGE)
        {
            // Generate new ID
            id = edges.size();
        }
        else
        {
            // Use provided ID (for original edges)
            id = edgeId;
        }

        if (shortcut)
        {
            edges.push_back(Edge{
                id,
                /*rev_id*/ -1, // fill later if you keep a downward mirror
                u, v,
                w,
                true,
                shortcut});
        }

        // if(!shortcut && edgeId != INVALID_EDGE) id = edgeId;

        edge_index[edge_key(u, v)] = id;
        // Upward adjacency for local neighbor iteration:
        upward_edges_adj[u][v] = {id, w};

        // Bucket by higher endpoint's rank for bottom-up customization:
        int r = rank[v];
        if ((int)buckets_by_rank.size() <= r)
            buckets_by_rank.resize(r + 1);
        buckets_by_rank[r].push_back(id);

        return id;
    }

    EdgeId ensure_upward_edge(NodeId a, NodeId b, Weight orig_w = INF_WEIGHT, EdgeId edgeId = INVALID_EDGE)
    {
        // Orient by rank
        NodeId u = (rank[a] < rank[b]) ? a : b;
        NodeId v = (u == a) ? b : a;

        // Sanity: u must be lower-rank
        // (Skip in release or use assert)
        // assert(rank[u] < rank[v]);

        const uint64_t k = edge_key(u, v);
        auto it = edge_index.find(k);
        if (it != edge_index.end())
        {
            // Already exists: return id and (optionally) merge original weight
            EdgeId id = it->second;

            return id;
        }

        // Not present: create new

        // Determine if this is a shortcut or original edge
        bool is_shortcut = (orig_w == INF_WEIGHT);

        // const Weight w = orig_w.value_or(INF_WEIGHT);

        return add_upward_edge(u, v, orig_w, is_shortcut, edgeId);
    }

    inline const Edge &get_edge(EdgeId id) const { return edges[id]; }

    void build_upward_adj_Lists()
    {
        edge_index.clear();
        buckets_by_rank.clear();
        upward_edges_adj.assign(nodes.size(), {});

        // cout << "Building upward adj lists from " << edges.size() << " edges" << endl;

        for (auto &e : edges)
        {
            NodeId a = e.src, b = e.trg;
            Weight w = e.cost;

            // cout << "Processing edge " << a << "->" << b << " with ID " << e.id << endl;

            // For original edges, pass the original weight and ID
            ensure_upward_edge(a, b, w, e.id);
        }

        // cout << "Finished building upward adj lists" << endl;
    }

    void print_upward_adj()
    {
        cout << "printing upward adj list.";
        for (NodeId src = 0; src < upward_edges_adj.size(); src++)
        {
            for (const auto &[trg, edgeInfo] : upward_edges_adj[src])
            {
                cout << "Upward Edge from " << src << " to " << trg << " (ID: " << edgeInfo.first << ", Cost: " << edgeInfo.second << ", is_shortcut: " << get_edge(edgeInfo.first).shortcut << ")\n";
            }
        }
    }

    void customize();
};
