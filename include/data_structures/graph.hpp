#pragma once
#include <unordered_map>
#include <vector>
#include "data_structures/node.hpp"
#include "data_structures/edge.hpp"
#include <stdexcept>
#include <iostream>

using namespace std;

struct Graph
{
    vector<Node> nodes; // size n, nodes[i].id == i
    vector<uint8_t> active;
    vector<Edge> edges; // flat edge storage                    // 1=active, 0=inactive
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;

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
    inline const Edge &get_edge(EdgeId id) const { return edges[id]; }
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
                edge1.cost = static_cast<Weight>(shortcut.cap);
                edge1.shortcut = true;
                edge1.sc = {
                    // get the edgeIds.
                    edge1.sc.e_uv = get_edge_by_src_dst(shortcut.u, shortcut.v).id,
                    edge1.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.w).id,
                    edge1.sc.middle = shortcut.v,
                };
                // set_edge();

                Edge edge2;
                edge2.src = shortcut.w;
                edge2.trg = shortcut.u;
                edge2.cost = static_cast<Weight>(shortcut.cap);
                edge2.shortcut = true;
                edge2.sc = {
                    edge2.sc.e_uv = get_edge_by_src_dst(shortcut.w, shortcut.v).id,
                    edge2.sc.e_vw = get_edge_by_src_dst(shortcut.v, shortcut.u).id,
                    edge2.sc.middle = shortcut.v,
                };
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
