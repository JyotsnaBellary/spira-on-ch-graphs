#pragma once

#include <enums.hpp>
#include <elimination-tree.hpp>

// using NodePQEntry = pair<Dist, NodeId>;

// using NodePQ = priority_queue<
//     NodePQEntry,
//     vector<NodePQEntry>,
//     greater<NodePQEntry>
// >;

class NodeScheduler {
    private:
        SearchMode mode;
        NodePQ* pq = nullptr;

        const EliminationTree* tree = nullptr;
        NodeId current = INVALID_NODE;

    public:
        explicit NodeScheduler(NodePQ& pq);

        NodeScheduler(const EliminationTree& tree, NodeId start);

        bool has_next() const;

        pair<Dist, NodeId> pop(const vector<Dist>& dist);
    Cost peek_cost() const;
        void push(Cost cost, NodeId node);
};