#include <algorithms/CCH.hpp>

CCH::CCH(Graph& graph) : graph(graph) {}

void CCH::preprocess() {
    
    compute_rank_order();
}

void CCH::compute_rank_order() {
    // Implementation of rank order computation


    compute_nested_dissection();

}

void CCH::compute_nested_dissection() {
    // Implementation of nested dissection
    //basically in this we use kahip first for the whole graph
    // get a seperator
    // write each parts to seperate files and keep calling again and again
    // until we reach a base case i.e. we get individual nodes

    // these individual nodes will be given higher rank priority
    // then the ones at the upper level and teh upper than that and so on. 


    // at each level, the nodes are basically ranked in any random order but they are all from same level. 

    // need to write a function to 




}

void CCH::add_shortcut(NodeId u, NodeId v, Weight w) {
    // Implementation of adding a shortcut
}

// void CCH::remove_shortcut(NodeId u, NodeId v) {
//     // Implementation of removing a shortcut
// }

const vector<pair<Shortcut, ShortcutOpType>>& CCH::get_shortcuts(NodeId nodeId) const {
    // Implementation of getting shortcuts for a node
    return shortcutsCache;
}
