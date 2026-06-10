#include "node-scheduler.hpp"

NodeScheduler::NodeScheduler(NodePQ& pq)
    : mode(SearchMode::PriorityQueue),
      pq(&pq) {}

NodeScheduler::NodeScheduler(const EliminationTree& tree, NodeId start)
    : mode(SearchMode::EliminationTree),
      tree(&tree),
      current(start){}

bool NodeScheduler::has_next() const
{
    if (mode == SearchMode::PriorityQueue)
        return !pq->empty();

    return current != INVALID_NODE;
}

void NodeScheduler::push(Cost cost, NodeId node)
{
    if (mode == SearchMode::PriorityQueue)
        pq->push({cost, node});
}

// Cost NodeScheduler::peek_cost() const
// {
//     if (mode == SearchMode::PriorityQueue)
//         return pq->top().first;

//     return 0;
// }

pair<Dist, NodeId> NodeScheduler::pop(const vector<Dist>& dist)
{
    if (mode == SearchMode::PriorityQueue)
    {
       auto entry = pq->top();
        pq->pop();
        return entry;
    }

    NodeId node = current;
    current = tree->get_parent(current);

    return {dist[node], node};
}