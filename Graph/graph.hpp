#pragma once

#include <vector>

#include "node.hpp"
#include "edge.hpp"
#include <iostream>
#include <algorithm>

struct Graph
{
    private:
        vector<Node> nodes;
        vector<Edge> edges;

        // Usual adjacency list that stores outgoing edges for each node
        vector<vector<EdgeId>> out_adjacency_list;

        // Adjacency list that stores incoming edges for each node
        vector<vector<EdgeId>> in_adjacency_list;

        bool valid_node(NodeId u) const;
        
    public: 
    
    // Constructors
    Graph();
    explicit Graph(int number_of_nodes);

    // Node Mutators
    void set_node(const Node &node);

    // Edge Mutator
    void set_edge(Edge &edge, Edge &rev_edge);

    const Node &get_node(NodeId &nodeId) const;
    const Edge &get_edge(EdgeId &edgeId) const;

    const vector<Node>& get_nodes() const;
    const vector<Edge>& get_edges() const;

    // Graph Properties
    int number_of_nodes() const;
    int number_of_edges() const;

    // Adds edge to out_adjacency_list
    void add_edge_adj(Edge &edge);

    // Query Neighbors of a node
    const vector<EdgeId> &get_out_neighbors(NodeId nodeId) const;
    const vector<EdgeId> &get_in_neighbors(NodeId nodeId) const;

    // Sort neighbors of a node by edge cost
    void sort_out_neighbors(NodeId nodeId);
    void sort_in_neighbors(NodeId nodeId);
   
    // sort neighbors of all nodes
    void sort_all_neighbors();

    // Simple print of adjacency list
    void print_adj_simple();
    
};