#pragma once
#include <unordered_map>
#include <vector>
#include <node.hpp>
#include <edge.hpp>
#include <types.hpp>
#include <stdexcept>
#include <iostream>

using namespace std;

struct CH_DijkstraResult
{
    vector<NodeId> path;
    vector<NodeId> ch_path;
    Dist total_cost;
    vector<EdgeId> edge_ids;
    vector<EdgeId> ch_edge_ids;
    int number_of_pops;
};

// This is a common struct is used to store with upper edges
// It is used for CH and CCH for query Processing
struct CH_Graph
{
    // Graph Utils
    CH_Graph() = default;
    vector<Node> nodes;
    vector<Edge> edges;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> adj;
    vector<unordered_map<NodeId, pair<EdgeId, Weight>>> upward_edges_adj;
    // vector<unordered_map<NodeId, pair<EdgeId, Weight>>> downward_edges_adj;
    // Usual adjacency list that stores outgoing edges for each node
    vector<vector<EdgeId>> out_adjacency_list;

    // Adjacency list that stores incoming edges for each node
    vector<vector<EdgeId>> in_adjacency_list;

    vector<int> rank;
    vector<bool> is_shortcut;
    int shortcuts = 0;

    // resize nodes, edges, active and adj
    CH_Graph(vector<Node> nodes_in_ch, vector<Edge> edges_in_ch, vector<int> rank_order, bool build_adj = false)
    {
        nodes = nodes_in_ch;
        edges = edges_in_ch;
        upward_edges_adj.resize(nodes.size());
        rank = rank_order;
        is_shortcut.resize(edges.size());
        out_adjacency_list.resize(nodes.size());
        in_adjacency_list.resize(nodes.size());
        // build upward and downward adj list
        if (build_adj) {
            build_upward_adj_Lists(rank_order);
        }
    }

    inline bool is_up(NodeId u, NodeId v) { return rank[v] > rank[u]; } // your convention

    int number_of_nodes() const { return static_cast<int>(nodes.size()); }
    inline const Edge &get_edge(EdgeId id) const { return edges.at(id); }
    inline const vector<Edge> &get_all_edges() const { return edges; }
    inline const int &get_number_of_shortcuts() const { return shortcuts; }

    // To query only for upper ranked neighbors
    const unordered_map<NodeId, pair<EdgeId, Weight>> &up_neighbors(NodeId node) const
    {
        if (node < 0 || node >= static_cast<int>(upward_edges_adj.size()))
        {
            throw out_of_range("Invalid node ID");
        }
        return upward_edges_adj[node];
    }
    inline const vector<Node> &get_all_nodes() const { return nodes; }

    inline int number_of_edges() const { return static_cast<int>(edges.size()); }

    bool valid_node(NodeId u) const
    {
        return u >= 0 && u < number_of_nodes();
    }

    // Build adjacency list with only upward edges
    void build_upward_adj_Lists(vector<int> rank_order)
    {
        for (Edge &edge : edges)
        {
            if (is_up(edge.src, edge.trg))
            {
                upward_edges_adj[edge.src][edge.trg] = {edge.id, edge.cost};
                if (edge.shortcut)
                {
                    shortcuts += 1;
                    is_shortcut[edge.id] = true;
                }
                add_to_out_adj(edge);
                add_to_in_adj(edge);
            }
        }
    }

    void set_upward_adj_lists(vector<vector<EdgeId>> &out_adj, vector<vector<EdgeId>> &in_adj) {
        out_adjacency_list = out_adj;
        in_adjacency_list = in_adj;
    }

    void add_to_out_adj(Edge &edge) {
        add_to_out_adj(edge, out_adjacency_list);
    }

    void add_to_out_adj(Edge &edge, vector<vector<EdgeId>> &adjacency_list)
    {
        if (!valid_node(edge.src))
            throw out_of_range("add_edge_adj: invalid node id " + to_string(edge.src));
        if (edge.id < 0 || edge.id >= static_cast<EdgeId>(edges.size()))
            throw out_of_range("add_edge_adj: invalid edge id " + to_string(edge.id));
        adjacency_list[edge.src].push_back(edge.id);
    }

    void add_to_in_adj(Edge &edge) {
        add_to_in_adj(edge, in_adjacency_list);
    }

    void add_to_in_adj(Edge &edge, vector<vector<EdgeId>> &adjacency_list)
    {
        if (!valid_node(edge.src))
            throw out_of_range("add_edge_adj: invalid node id " + to_string(edge.src));
        if (edge.id < 0 || edge.id >= static_cast<EdgeId>(edges.size()))
            throw out_of_range("add_edge_adj: invalid edge id " + to_string(edge.id));
        adjacency_list[edge.trg].push_back(edge.id);
    }

    void print_upward_adj()
    {
        for (NodeId src = 0; src < upward_edges_adj.size(); src++)
        {
            for (const auto &[trg, edgeInfo] : upward_edges_adj[src])
            {
                cout << "Upward Edge from " << src << " to " << trg << " (ID: " << edgeInfo.first << ", Cost: " << edgeInfo.second << ")\n";
            }
        }
    }

    // Simple print of adjacency list
    void print_adj_out()
    {
        for (NodeId u = 0; u < static_cast<NodeId>(nodes.size()); ++u)
        {
            cout << "[" << u << "] : [";
            const auto &A = out_adjacency_list[u];
            for (size_t i = 0; i < A.size(); ++i)
            {
                EdgeId eid = A[i];
                cout << edges[eid].trg;
                if (i + 1 < A.size())
                    cout << " ";
            }
            cout << "]\n";
        }
    }
    void print_adj_in()
    {
        cout << "Incoming adjacency list:\n";
        for (NodeId u = 0; u < static_cast<NodeId>(nodes.size()); ++u)
        {
            cout << "[" << u << "] : [";
            const auto &A = in_adjacency_list[u];
            for (size_t i = 0; i < A.size(); ++i)
            {
                EdgeId eid = A[i];
                cout << edges[eid].src;
                if (i + 1 < A.size())
                    cout << " ";
            }
            cout << "]\n";
        }
    }
    // helper for recursive unpacking of edges after CH and CCh Query
    inline void append_unpacked(EdgeId eid,
                                vector<EdgeId> &edge_out,
                                vector<NodeId> &node_out,
                                bool first = false)
    {
        const Edge &e = get_edge(eid);
        // Recursively unpack if it's a shortcut
        if (e.shortcut)
        {
            append_unpacked(e.sc.e_uv, edge_out, node_out, first);
            append_unpacked(e.sc.e_vw, edge_out, node_out, false);
        }
        else
        {
            edge_out.push_back(eid);
            if (first)
                node_out.push_back(e.src);
            node_out.push_back(e.trg);
        }
    }

    // Main function to unpack path returned by Ch Dijkstra
    CH_DijkstraResult unpack_shortcuts(CH_DijkstraResult ch_dijkstraResult)
    {
        vector<EdgeId> edge_out;
        vector<EdgeId> node_out;
        edge_out.reserve(ch_dijkstraResult.edge_ids.size() * 2); // rough guess; grows as needed
        bool first = true;
        for (EdgeId eid : ch_dijkstraResult.ch_edge_ids)
        {
            append_unpacked(eid, edge_out, node_out, first);
            first = false; // only put the src once
        }

        ch_dijkstraResult.edge_ids = move(edge_out);
        ch_dijkstraResult.path = move(node_out);

        return ch_dijkstraResult;
    }

    // Query neighbors of node
    const vector<EdgeId> get_out_neighbors(NodeId nodeId) const
    {
        return out_adjacency_list[nodeId];
    }

    // Query neighbors of node that have incoming edges to it
    const vector<EdgeId> get_in_neighbors(NodeId nodeId) const
    {
        return in_adjacency_list[nodeId];
    }

    // Sort neighbors of a node by edge cost that has outgoing edges from it
    void sort_out_neighbors(NodeId nodeId)
    {
        if (nodeId < 0 || nodeId >= number_of_nodes())
            return;
        auto &neighbor_list = out_adjacency_list[nodeId];
        sort(neighbor_list.begin(), neighbor_list.end(),
             [&](EdgeId a, EdgeId b)
             {
                 return edges[a].cost < edges[b].cost; // optional tiebreak
             });
    }

    // Sort neighbors of a node by edge cost that have incoming edge to it
    void sort_in_neighbors(NodeId nodeId)
    {
        if (nodeId < 0 || nodeId >= number_of_nodes())
            return;
        auto &neighbor_list = in_adjacency_list[nodeId];
        sort(neighbor_list.begin(), neighbor_list.end(),
             [&](EdgeId a, EdgeId b)
             {
                 return edges[a].cost < edges[b].cost; // optional tiebreak
             });
    }

    // sort neighbors of all nodes
    void sort_all_neighbors()
    {
        for (NodeId nodeId = 0; nodeId < number_of_nodes(); nodeId++)
        {
            sort_out_neighbors(nodeId);
            sort_in_neighbors(nodeId);
        }
    }

    vector<NodeId> find_reachable_nodes(NodeId src)
    {
        vector<NodeId> reachable_nodes;

        vector<bool> reachable(number_of_nodes(), false);
        queue<NodeId> pq;

        reachable[src] = true;
        reachable_nodes.push_back(src);
        pq.push(src);

        while (!pq.empty())
        {
            NodeId u = pq.front();
            pq.pop();

            for (const EdgeId edge_id : get_out_neighbors(u))
            {
                const NodeId v = edges[edge_id].trg;
                if (!reachable[v])
                {
                    reachable[v] = true;
                    reachable_nodes.push_back(v);
                    pq.push(v);
                }
            }
        }

        return reachable_nodes;
    }

    Query_Graph_Type get_upward_graph(NodeId src)
    {
        vector<NodeId> reachable_nodes = find_reachable_nodes(src);
        int edge_count = 0;
        vector<vector<EdgeId>> query_out_adjacency_list(number_of_edges());
        vector<vector<EdgeId>> query_in_adjacency_list(number_of_edges());

        Query_Graph_Type query_graph_info;

        for (NodeId node_id: reachable_nodes) {
            for (const EdgeId edgeId: get_out_neighbors(node_id)) {
                const Edge &edge = get_edge(edgeId);
                if (is_up(edge.src, edge.trg)) {
                    edge_count++;
                    query_out_adjacency_list[edge.src].push_back(edgeId);
                    query_in_adjacency_list[edge.trg].push_back(edgeId);
                }
            }
        }

        query_graph_info.out_adjacency_list = move(query_out_adjacency_list);
        query_graph_info.in_adjacency_list = move(query_in_adjacency_list);
        query_graph_info.reachable_nodes = move(reachable_nodes);
        query_graph_info.edge_count = edge_count;

        return query_graph_info;
    }
};
