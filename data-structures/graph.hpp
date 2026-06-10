#pragma once
#include <unordered_map>
#include <vector>
#include <node.hpp>
#include <edge.hpp>
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <assert.h>

using namespace std;

class Graph
{
    public:
        vector<Node> nodes; // size n, nodes[i].id == i
        vector<uint8_t> active;
        vector<Edge> edges; // flat edge storage                    // 1=active, 0=inactive
        vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;
        unordered_map<uint64_t, size_t> edge_pos;

        // Usual adjacency list that stores outgoing edges for each node
        vector<vector<EdgeId>> out_adjacency_list;

        // Adjacency list that stores incoming edges for each node
        vector<vector<EdgeId>> in_adjacency_list;

        Graph();

    // resize nodes, edges, active and adj
        Graph(int number_of_nodes);
    // {
    //     nodes.resize(number_of_nodes);
    //     active.resize(number_of_nodes, 1);
    //     adj.resize(number_of_nodes);
    //     out_adjacency_list.resize(number_of_nodes);
    //     in_adjacency_list.resize(number_of_nodes);
    // }

        // explicit Graph(size_t n);
    //     : nodes(n), active(n, 1), adj(n)
    // {
    //     for (NodeId i = 0; i < static_cast<NodeId>(n); ++i)
    //     {
    //         nodes[i].id = i;
    //         nodes[i].latitude = nodes[i].longitude = 0.0;
    //     }
    // }

    // Node helper functions
    int number_of_nodes() const;
    //   { return static_cast<int>(nodes.size()); }
    const Node &get_node(NodeId id) const;
    //  { return nodes[id]; }
    void set_node(NodeId id, const Node &node);
    // { nodes[id] = node; }
    const vector<Node> &get_all_nodes() const;
    //  { return nodes; }

    // const vector<NodeId> get_all_node_ids();
    // {
    //     vector<NodeId> nodes(number_of_nodes());
    //     iota(nodes.begin(), nodes.end(), 0);
    //     return nodes;
    // };

    NodeId get_any_active_node() const;
    // {
    //     for (NodeId u = 0; u < number_of_nodes(); ++u)
    //     {
    //         if (is_active(u))
    //             return u;
    //     }
    //     return INVALID_NODE; // no active node found
    // }

    // Edge Helper Functions
    static uint64_t key_directed(NodeId u, NodeId v);
    // {
    //     return (uint64_t)u << 32 | (uint32_t)v;
    // }
    // --- lookup helper ---
    size_t edge_index(NodeId src, NodeId dst) const;
    // {
    //     auto it = edge_pos.find(key_directed(src, dst));
    //     if (it == edge_pos.end())
    //         throw out_of_range("Edge not found");
    //     return it->second;
    // }

    Edge &get_edge(EdgeId id);
    // { return edges.at(id); }

    // read-only
    const Edge &get_edge(NodeId src, NodeId dst) const;
    // {
    //     return edges[edge_index(src, dst)];
    // }

    // returns true if edge exists
    bool edge_exists(NodeId src, NodeId dst) const;
    // {
    //     return edge_pos.find(key_directed(src, dst)) != edge_pos.end();
    // }

    Edge &get_edge(NodeId src, NodeId dst);
    // {
    //     return edges[edge_index(src, dst)];
    // }

    // Query edge by src, dst
    const Edge &get_edge_by_src_dst(NodeId src, NodeId dst) const;
    // {
    //     auto it = adj[src].find(dst);
    //     if (it != adj[src].end())
    //     {
    //         return edges[it->second.first];
    //     }
    //     throw runtime_error("Edge not found");
    // }
    const vector<Edge> &get_all_edges() const;
    //  { return edges; }

    // Set Edge, Mainly used while creating shortcuts
    void set_edge(Edge &edge, Edge &rev_edge);
    // {
    //     EdgeId eid = static_cast<EdgeId>(edges.size());
    //     edge.id = eid; // ensure edge has a valid id
    //     edge.rev_id = eid + 1;
    //     edges.emplace_back(edge);

    //     rev_edge.id = eid + 1; // ensure reverse edge has a valid id
    //     rev_edge.rev_id = eid;
    //     edges.emplace_back(rev_edge);
    //     edge_pos[key_directed(edge.src, edge.trg)] = edge.id;
    //     edge_pos[key_directed(rev_edge.src, rev_edge.trg)] = rev_edge.id;
    //     add_edge_adj(eid, edge.src, edge.trg, edge.cost);
    //     add_edge_adj(eid + 1, rev_edge.src, rev_edge.trg, rev_edge.cost);
    // }

    int number_of_edges() const;
    //  { return static_cast<int>(edges.size()); }
    bool is_active(NodeId v) const;
    // { return active[v] != 0; }
    void deactivate(NodeId v);
    // { active[v] = 0; }

    // Adjacency list helper functions
    int num_active_neighbors(NodeId u) const;
    // {
    //     int count = 0;
    //     for (const auto &[v, edgeInfo] : adj[u])
    //     {
    //         if (is_active(v))
    //             count++;
    //     }
    //     return count;
    // }

    void build_adjacency_list(const vector<Edge> *customEdges = nullptr);
    // {
    //     const auto &es = customEdges ? *customEdges : edges;

    //     adj.resize(nodes.size());
    //     for (const auto &edge : es)
    //     {
    //         adj[edge.src][edge.trg] = {edge.id, edge.cost};
    //     }
    // }

    void print_adj();
    // {
    //     for (NodeId src = 0; src < adj.size(); src++)
    //     {
    //         for (const auto &[trg, edgeInfo] : adj[src])
    //         {
    //             cout << "Upward Edge from " << src << " to " << trg << " (ID: " << edgeInfo.first << ", Cost: " << edgeInfo.second << ")\n";
    //         }
    //     }
    // }

    // add individual edge to adjacency list
    EdgeId add_edge_adj(EdgeId edge_id, NodeId u, NodeId v, Weight w);
    // {
    //     adj[u][v] = {edge_id, w};
    //     return edge_id;
    // }

    const unordered_map<NodeId, pair<EdgeId, Weight>> &neighbors(NodeId u) const;
    // {
    //     return adj[u];
    // }

    const void build_out_in_adjacency_lists();
    // {
    //     for (const auto &edge : edges)
    //     {
    //         out_adjacency_list[edge.src].push_back(edge.id);
    //         in_adjacency_list[edge.trg].push_back(edge.id);
    //     }
    // }

    // Query neighbors of node
    const vector<EdgeId> get_out_neighbors(NodeId nodeId) const;
    // {
    //     return out_adjacency_list[nodeId];
    // }

    // Query neighbors of node that have incoming edges to it
    const vector<EdgeId> get_in_neighbors(NodeId nodeId) const;
    // {
    //     return in_adjacency_list[nodeId];
    // }

    // Sort neighbors of a node by edge cost that has outgoing edges from it
    void sort_out_neighbors(NodeId nodeId);
    // {
    //     if (nodeId < 0 || nodeId >= number_of_nodes())
    //         return;
    //     auto &neighbor_list = out_adjacency_list[nodeId];
    //     sort(neighbor_list.begin(), neighbor_list.end(),
    //          [&](EdgeId a, EdgeId b)
    //          {
    //              return edges[a].cost < edges[b].cost; // optional tiebreak
    //          });
    // }

    // Sort neighbors of a node by edge cost that have incoming edge to it
    void sort_in_neighbors(NodeId nodeId);
    // {
    //     if (nodeId < 0 || nodeId >= number_of_nodes())
    //         return;
    //     auto &neighbor_list = in_adjacency_list[nodeId];
    //     sort(neighbor_list.begin(), neighbor_list.end(),
    //          [&](EdgeId a, EdgeId b)
    //          {
    //              return edges[a].cost < edges[b].cost; // optional tiebreak
    //          });
    // }

    // sort neighbors of all nodes
    void sort_all_neighbors();
    // {
    //     for (NodeId nodeId = 0; nodeId < number_of_nodes(); nodeId++)
    //     {
    //         sort_out_neighbors(nodeId);
    //         sort_in_neighbors(nodeId);
    //     }
    // }

    // Get sorted list of upper neighor node ids
    vector<NodeId> get_sorted_higher_neighbors(NodeId nodeId, vector<int> &node_rank) const;
    // {
    //     vector<NodeId> up_neighbors;
    //     up_neighbors.reserve(adj[nodeId].size());

    //     // Iterate the adjacency map directly;.
    //     for (const auto &[neighbor_id, _] : adj[nodeId])
    //     {
    //         // Guard against corrupted keys
    //         if (neighbor_id < 0 || static_cast<size_t>(neighbor_id) >= node_rank.size())
    //             continue;
    //         if (node_rank[neighbor_id] > node_rank[nodeId])
    //             up_neighbors.push_back(neighbor_id);
    //     }

    //     sort(up_neighbors.begin(), up_neighbors.end(),
    //          [&](NodeId a, NodeId b)
    //          { return node_rank[a] < node_rank[b]; });

    //     return up_neighbors;
    // }

    // Function to add shortcuts to the graph
    int add_shortcuts(vector<pair<Shortcut, ShortcutOpType>> &shortcuts);
    // {
    //     // Add shortcuts for the given node
    //     int count = 0;
    //     for (const auto &[shortcut, shortcutOpType] : shortcuts)
    //     {
    //         // create a seperate function for them.
    //         if (shortcutOpType == ShortcutOpType::ADD)
    //         {
    //             Edge edge1;
    //             edge1.src = shortcut.u;
    //             edge1.trg = shortcut.w;

    //             edge1.cost = static_cast<Weight>(shortcut.cap);
    //             edge1.shortcut = true;
    //             edge1.sc = {
    //                 // get the edgeIds.
    //                 edge1.sc.middle = shortcut.v,
    //                 edge1.sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
    //                 edge1.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,

    //             };

    //             Edge edge2;
    //             edge2.src = shortcut.w;
    //             edge2.trg = shortcut.u;
    //             edge2.cost = static_cast<Weight>(shortcut.cap);
    //             edge2.shortcut = true;
    //             edge2.sc = {
    //                 edge2.sc.middle = shortcut.v,
    //                 edge2.sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
    //                 edge2.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,

    //             };

    //             set_edge(edge1, edge2);

    //             count += 1;
    //         }
    //         // If the shortcut is replacing an original edge
    //         else if (shortcutOpType == ShortcutOpType::REPLACE)
    //         {
    //             // Find and update the existing edge. but uw and wu
    //             auto it = adj[shortcut.u].find(shortcut.w);
    //             if (it != adj[shortcut.u].end())
    //             {
    //                 EdgeId eid = it->second.first;
    //                 edges[eid].cost = static_cast<Weight>(shortcut.cap);
    //                 it->second.second = static_cast<Weight>(shortcut.cap); // update cost in adj
    //                 edges[eid].shortcut = true;
    //                 edges[eid].sc = {
    //                     // get the edgeIds
    //                     edges[eid].sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
    //                     edges[eid].sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,
    //                     edges[eid].sc.middle = shortcut.v,
    //                 };
    //             }

    //             auto it1 = adj[shortcut.w].find(shortcut.u);
    //             if (it1 != adj[shortcut.w].end())
    //             {
    //                 EdgeId eid = it1->second.first;
    //                 edges[eid].cost = static_cast<Weight>(shortcut.cap);
    //                 it1->second.second = static_cast<Weight>(shortcut.cap); // update cost in adj
    //                 edges[eid].shortcut = true;
    //                 edges[eid].sc = {
    //                     // get the edgeIds
    //                     edges[eid].sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
    //                     edges[eid].sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,
    //                     edges[eid].sc.middle = shortcut.v,
    //                 };
    //             }
    //         }
    //     }

    //     return count;
    // }

    // Update shortcut Information before storing in cache
    void update_shortcut_info(EdgeId edgeId, EdgeId rev_edgeId, ShortcutInfo shortcutInfo, Weight cost, bool is_shortcut);
    // {
    //     if (is_shortcut && edges[edgeId].replaced == false)
    //     {
    //         edges[edgeId].replaced = true;
    //         edges[edgeId].shortcut = true;
    //         edges[edgeId].original_cost = edges[edgeId].cost;

    //         edges[rev_edgeId].replaced = true;
    //         edges[rev_edgeId].shortcut = true;
    //         edges[rev_edgeId].original_cost = edges[rev_edgeId].cost;
    //     }
    //     edges[edgeId].sc.e_uv = shortcutInfo.e_uv;
    //     edges[edgeId].sc.e_vw = shortcutInfo.e_vw;
    //     edges[edgeId].sc.middle = shortcutInfo.middle;
    //     edges[edgeId].cost = cost;

    //     // Update Reverse Edge
    //     edges[rev_edgeId].sc.e_uv = get_edge(shortcutInfo.e_vw).rev_id;
    //     edges[rev_edgeId].sc.e_vw = get_edge(shortcutInfo.e_uv).rev_id;
    //     edges[rev_edgeId].sc.middle = shortcutInfo.middle;
    //     edges[rev_edgeId].cost = cost;
    // }
};
