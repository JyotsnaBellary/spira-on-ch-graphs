#pragma once
#include "core/types.hpp"

struct ShortcutInfo {
    NodeId middle = INVALID_NODE;   // v contracted between u and w
    EdgeId e_uv   = INVALID_EDGE;   // optional: edge id u->v
    EdgeId e_vw   = INVALID_EDGE;   // optional: edge id v->w
};

struct Edge { 
    EdgeId id = INVALID_EDGE;          // unique within current graph view
    EdgeId rev_id = INVALID_EDGE;      // id of reverse edge (if any)
    NodeId src = INVALID_NODE;
    NodeId trg = INVALID_NODE;
    Weight cost = 1;    // default per your format
    bool   shortcut = false;
    bool   active = true;
    ShortcutInfo sc;    // valid iff shortcut==true


    // optional lineage (debugging)
    EdgeId replaced    = false;  // direct edge this shortcut replaced
    EdgeId original_cost = cost;

    Edge() = default;
    Edge(EdgeId eid, EdgeId rev_eid, NodeId u, NodeId v, Weight w, bool active=true, bool shortcut=false)
        : id(eid), rev_id(rev_eid), src(u), trg(v), cost(w), active(active), shortcut(shortcut) {}
};
