#include <core/types.hpp>
#include <utility>
#include <vector>
#include <queue>
#include <data_structures/graph.hpp>
#include <climits>

//not used

// // Helper function to get valid node from PQ
// pair<int, NodeId> pop_valid_node(priority_queue<pair<int, NodeId>, 
//                                 vector<pair<int, NodeId>>, 
//                                 greater<pair<int, NodeId>>>& pq, Graph& graph, vector<int>& currentEdgeDiffs) {
//     while (!pq.empty()) {
//         auto top = pq.top();
//         pq.pop();
        
//         // Check if this entry is still valid
//         if (graph.is_active(top.second) && currentEdgeDiffs[top.second] == top.first) {
//             return top;
//         }
//     }
//     return {INT_MAX, INVALID_NODE};
// }


