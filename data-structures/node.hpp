#pragma once
#include <types.hpp>

struct Node {
    NodeId id;   // internal id: 0..n-1
    double latitude;  // degrees
    double longitude;  // degrees
};
