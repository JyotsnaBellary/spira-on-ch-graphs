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
using Weight = double;
using Dist = double;
using Cost = double;

// changed from int to dist
using PQItem = pair<Dist, NodeId>;
using NodePQ = priority_queue<PQItem, vector<PQItem>, greater<PQItem>>;
using EdgePQItem = pair<Cost, EdgeId>;
using EdgePQ = priority_queue<EdgePQItem, vector<EdgePQItem>, greater<EdgePQItem>>;

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
constexpr Weight INF_WEIGHT   = numeric_limits<Weight>::infinity();
constexpr Dist INF = numeric_limits<Weight>::infinity(); 
constexpr Cost INF_COST   = numeric_limits<Cost>::infinity();

constexpr Shortcut INVALID_SHORTCUT{
    INVALID_NODE, INVALID_NODE, INVALID_NODE, 0, 
    INVALID_EDGE, INVALID_EDGE
};

enum class ShortcutOpType { ADD, REPLACE, SKIP };

struct Ordering {
    // 1-based: node_of_rank[r] = node, rank_of_node[v] = r
    vector<NodeId> node_of_rank, rank_of_node;
};

