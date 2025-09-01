#pragma once
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>
#include <functional>
#include <utility>

using namespace std;
    
using NodeId = int32_t;
using EdgeId = int32_t;
//do I even need this anywhere?
using Weight = int32_t;
using Dist = long long;
using PQItem = pair<int, NodeId>;
using NodePQ = priority_queue<PQItem, vector<PQItem>, greater<PQItem>>;

struct Shortcut {
    NodeId u;       // left neighbor
    NodeId w;       // right neighbor
    NodeId v;       // banned/middle node
    Dist   cap;     // B = c(u,v) + c(v,w)
    EdgeId uv;      // edge id for u->v (child1)
    EdgeId vw;      // edge id for v->w (child2)
};

// Sentinel values
constexpr NodeId INVALID_NODE = numeric_limits<NodeId>::max();
constexpr EdgeId INVALID_EDGE = numeric_limits<EdgeId>::max();
constexpr Weight INF_WEIGHT   = numeric_limits<Weight>::max() / 4;
constexpr Dist INF = numeric_limits<Dist>::max() / 4; 
constexpr Shortcut INVALID_SHORTCUT{
    INVALID_NODE, INVALID_NODE, INVALID_NODE, 0, 
    INVALID_EDGE, INVALID_EDGE
};

enum class ShortcutOpType { ADD, REPLACE, SKIP };

struct Ordering {
    // 1-based: node_of_rank[r] = node, rank_of_node[v] = r
    vector<NodeId> node_of_rank, rank_of_node;
};

struct ShortcutRef {
    EdgeId e_uv;
    EdgeId e_vw;
    NodeId v;

};


