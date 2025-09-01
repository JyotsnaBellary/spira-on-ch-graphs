#pragma once
#include <unordered_map>
#include <vector>
#include "data_structures/node.hpp"
#include "data_structures/edge.hpp"
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <assert.h>

using namespace std;

struct Graph
{
    vector<Node> nodes; // size n, nodes[i].id == i
    vector<uint8_t> active;
    vector<Edge> edges; // flat edge storage                    // 1=active, 0=inactive
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;
    unordered_map<uint64_t, size_t> edge_pos;
    static inline uint64_t key_directed(NodeId u, NodeId v) {
        return (uint64_t)u << 32 | (uint32_t)v;
    }
    Graph() = default;

    // resize nodes, edges, active and adj
    Graph(int number_of_nodes)
    {
        nodes.resize(number_of_nodes);
        // edges.resize(number_of_edges*2); //will cause a problem when I add shortcuts
        active.resize(number_of_nodes, 1);
        adj.resize(number_of_nodes);
    }

    explicit Graph(size_t n)
        : nodes(n), active(n, 1), adj(n)
    {
        for (NodeId i = 0; i < static_cast<NodeId>(n); ++i)
        {
            nodes[i].id = i;
            nodes[i].lat = nodes[i].lon = 0.0;
        }
    }

    // add getter and setters
    inline const Node &get_node(NodeId id) const { return nodes[id]; }
    inline void set_node(NodeId id, const Node &node) { nodes[id] = node; }
    inline const vector<Node> &get_all_nodes() const { return nodes; }
    inline const vector<NodeId> get_all_node_ids() {
        vector<NodeId> nodes(num_nodes());
        iota(nodes.begin(), nodes.end(), 0);
        return nodes;
 
    };
    inline const Edge &get_edge(EdgeId id) const { return edges[id]; }
    // inline bool edge_exists(NodeId src, NodeId dst) const
    // {
    //     auto it = adj[src].find(dst);
    //     return it != adj[src].end();
    // }

    inline bool edge_exists(NodeId src, NodeId dst) const {
    return edge_pos.find(key_directed(src, dst)) != edge_pos.end();
}

    inline const Edge& get_edge(NodeId src, NodeId dst) const {
        auto it = edge_pos.find(key_directed(src, dst));
        if (it == edge_pos.end()) throw runtime_error("Edge not found");
        return edges[it->second];
    }
    inline const Edge &get_edge_by_src_dst(NodeId src, NodeId dst) const
    {
        auto it = adj[src].find(dst);
        if (it != adj[src].end())
        {
            return edges[it->second.first];
        }
        throw runtime_error("Edge not found");
    }
    inline const vector<Edge> &get_all_edges() const { return edges; }
    inline void set_edge(Edge &edge, Edge &rev_edge)
    {
        EdgeId eid = static_cast<EdgeId>(edges.size());
        edge.id = eid; // ensure edge has a valid id
        edge.rev_id = eid + 1;
        edges.emplace_back(edge);

        rev_edge.id = eid + 1; // ensure reverse edge has a valid id
        rev_edge.rev_id = eid;
        edges.emplace_back(rev_edge);
        edge_pos[key_directed(edge.src, edge.trg)] = edge.id;
        edge_pos[key_directed(rev_edge.src, rev_edge.trg)] = rev_edge.id;
        add_edge_adj(eid, edge.src, edge.trg, edge.cost);
        add_edge_adj(eid + 1, rev_edge.src, rev_edge.trg, rev_edge.cost);
    }

    inline int num_nodes() const { return static_cast<int>(nodes.size()); }
    inline int num_edges() const { return static_cast<int>(edges.size()); }
    inline bool is_active(NodeId v) const { return active[v] != 0; }
    inline void deactivate(NodeId v) { active[v] = 0; }
    int num_active_neighbors(NodeId u) const
    {
        int count = 0;
        for (const auto &[v, edgeInfo] : adj[u])
        {
            if (is_active(v))
                count++;
        }
        return count;
    }
    void build_adjacency_list(const vector<Edge> *customEdges = nullptr)
    {
        const auto &es = customEdges ? *customEdges : edges;

        adj.resize(nodes.size());
        for (const auto &edge : es)
        {
            adj[edge.src][edge.trg] = {edge.id, edge.cost};
        }
    }

    // add edge to adjacency list
    EdgeId add_edge_adj(EdgeId edge_id, NodeId u, NodeId v, Weight w)
    {
        // EdgeId eid = edges.size();
        // edges.emplace_back(eid, u, v, w, is_shortcut);
        adj[u][v] = {edge_id, w};
        return edge_id;
    }

    void update_edge(NodeId u, NodeId v, Weight w)
    {
        auto it = adj[u].find(v);
        if (it != adj[u].end())
        {
            EdgeId eid = it->second.first;
            edges[eid].cost = w;
            it->second.second = w; // update cost in adj
        }

        // edit_edge_adj(u, v, w);
    }

    // void print_adj_list() const {
    //     for (NodeId u = 0; u < num_nodes(); ++u) {
    //         if (is_active(u)) {
    //             cout << "Node " << u << ": ";
    //             for (const auto& [v, edgeInfo] : adj[u]) {
    //                 cout << "(to " << v << ", cost " << edgeInfo.second << ") ";
    //             }
    //             cout << endl;
    //         }
    //     }
    // }

    const unordered_map<NodeId, pair<EdgeId, Weight>> &neighbors(NodeId u) const
    {
        return adj[u];
    }

vector<NodeId> get_sorted_higher_neighbors(NodeId nodeId, vector<int>& node_rank) const {
    // Basic sanity checks
    // assert(v >= 0 && static_cast<size_t>(v) < adj.size());
    // // cout << node_rank.size()<< endl;
    // // cout << num_nodes() << endl;

    // assert(node_rank.size() == num_nodes()); // rank defined for every node


    vector<NodeId> up_neighbors;
    up_neighbors.reserve(adj[nodeId].size());

    // Iterate the adjacency map directly; neighbors(v) is fine too.
    for (const auto& [neighbor_id, _] : adj[nodeId]) {
        // Guard against corrupted keys
        if (neighbor_id < 0 || static_cast<size_t>(neighbor_id) >= node_rank.size()) continue;
        // if (edge_exists(nodeId, neighbor_id)){
        //     if(get_edge_by_src_dst(nodeId, neighbor_id).shortcut) continue;}
        if (node_rank[neighbor_id] > node_rank[nodeId]) up_neighbors.push_back(neighbor_id);
    }

    sort(up_neighbors.begin(), up_neighbors.end(),
              [&](NodeId a, NodeId b){ return node_rank[a] < node_rank[b]; });

    return up_neighbors;
}

    int add_shortcuts(vector<pair<Shortcut, ShortcutOpType>> &shortcuts)
    {
        // Add shortcuts for the given node
        int count = 0;
        for (const auto &[shortcut, shortcutOpType] : shortcuts)
        {
            // create a seperate function for them.
            if (shortcutOpType == ShortcutOpType::ADD)
            {
                Edge edge1;
                edge1.src = shortcut.u;
                edge1.trg = shortcut.w;
                // cout << "Creating shortcut edge: " << edge1.src << " -> " << edge1.trg << " and middle node: " << shortcut.v << endl;

                edge1.cost = static_cast<Weight>(shortcut.cap);
                edge1.shortcut = true;
                edge1.sc = {
                    // get the edgeIds.
                    edge1.sc.middle = shortcut.v,
                    edge1.sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
                    edge1.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,

                };
                // set_edge();

                Edge edge2;
                edge2.src = shortcut.w;
                edge2.trg = shortcut.u;
                // cout << "Creating reverse shortcut edge: " << edge2.src << " -> " << edge2.trg << " and middle node: " << shortcut.v << endl;
                edge2.cost = static_cast<Weight>(shortcut.cap);
                edge2.shortcut = true;
                edge2.sc = {
                    edge2.sc.middle = shortcut.v,
                    edge2.sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
                    edge2.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,

                };
                // cout << "Need shortcut edges: " << edge1.src << " -> " << edge1.trg << " and " << edge2.src << " -> " << edge2.trg << endl;
                //print the sc
                // cout << "Shortcut 1: " << edge1.sc.e_uv << ", " << edge1.sc.e_vw << ", " << edge1.sc.middle << endl;
                // cout << "Shortcut 2: " << edge2.sc.e_uv << ", " << edge2.sc.e_vw << ", " << edge2.sc.middle << endl;
                set_edge(edge1, edge2);

                count += 1;
            }
            else if (shortcutOpType == ShortcutOpType::REPLACE)
            {
                // Find and update the existing edge. but uw and wu
                auto it = adj[shortcut.u].find(shortcut.w);
                if (it != adj[shortcut.u].end())
                {
                    EdgeId eid = it->second.first;
                    edges[eid].cost = static_cast<Weight>(shortcut.cap);
                    it->second.second = static_cast<Weight>(shortcut.cap); // update cost in adj
                    edges[eid].shortcut = true;
                    edges[eid].sc = {
                        // get the edgeIds
                        edges[eid].sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
                        edges[eid].sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,
                        edges[eid].sc.middle = shortcut.v,
                    };
                }

                auto it1 = adj[shortcut.w].find(shortcut.u);
                if (it1 != adj[shortcut.w].end())
                {
                    EdgeId eid = it1->second.first;
                    edges[eid].cost = static_cast<Weight>(shortcut.cap);
                    it->second.second = static_cast<Weight>(shortcut.cap); // update cost in adj
                    edges[eid].shortcut = true;
                    edges[eid].sc = {
                        // get the edgeIds
                        edges[eid].sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
                        edges[eid].sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,
                        edges[eid].sc.middle = shortcut.v,
                    };
                }
            }
        }

        return count;
    }

    NodeId get_any_active_node() const
    {
        for (NodeId u = 0; u < num_nodes(); ++u)
        {
            if (is_active(u))
                return u;
        }
        return INVALID_NODE; // no active node found
    }
};
