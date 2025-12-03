#pragma once 
#include <types.hpp>

struct Edge {
    EdgeId id = INVALID_EDGE;
    NodeId src = INVALID_NODE;
    NodeId trg = INVALID_NODE;
    Cost cost = INF_COST;
};