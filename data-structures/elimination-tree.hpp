#pragma once
#include <ch-graph.hpp>
#include <types.hpp>
#include <vector>

using namespace std;

class EliminationTree
{
    private:
        vector<EdgeId> parent;
        const CH_Graph& graph;

    public: 
        explicit EliminationTree(const CH_Graph& graph);
        void build();
        bool has_parent(NodeId node) const;
        EdgeId get_parent_edge(NodeId node) const;
        NodeId get_parent(NodeId node) const;
        vector<NodeId> path_to_root(NodeId source) const;
        void print_elimination_tree() const;
};