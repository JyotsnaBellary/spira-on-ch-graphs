#pragma once
#include "core/types.hpp"

struct Node {
    NodeId id;   // internal id: 0..n-1
    double lat;  // degrees
    double lon;  // degrees
};
