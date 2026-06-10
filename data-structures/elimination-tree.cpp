#include <elimination-tree.hpp>

EliminationTree::EliminationTree(const CH_Graph& graph): graph(graph)
{
    parent.resize(graph.number_of_nodes(), INVALID_EDGE);
    cout << "Building elimination tree...\n";
    build();
}

void EliminationTree::build() {
    for (NodeId node_id = 0; node_id < graph.number_of_nodes(); ++node_id) {        
        // find lowest ranked parent among up neighbors
        EdgeId parent_edge_id = graph.get_lowest_ranked_upward_neighbor(node_id);
        cout << "Node " << node_id << " has parent edge ID: " << parent_edge_id << endl;
        parent[node_id] = parent_edge_id;
    } 

}

bool EliminationTree::has_parent(NodeId node_id) const {
    return parent[node_id] != INVALID_EDGE;
}

EdgeId EliminationTree::get_parent_edge(NodeId node_id) const {
    return parent[node_id];
}

NodeId EliminationTree::get_parent(NodeId node_id) const {
    if (!has_parent(node_id)) 
        return INVALID_NODE; // or handle the case where there's no parent

    EdgeId parent_edge_id = get_parent_edge(node_id);
    const Edge& parent_edge = graph.get_edge(parent_edge_id);
    return parent_edge.trg; // parent is the target of the edge from node_id
    
}

vector<NodeId> EliminationTree::path_to_root(NodeId source) const
{
    vector<NodeId> path;

    NodeId current = source;

    while (current != INVALID_NODE)
    {
        path.push_back(current);
        current = get_parent(current);
    }

    return path;
}

void EliminationTree::print_elimination_tree() const {
    cout << "Elimination Tree (Node -> Parent Node):\n";
    for (NodeId node_id = 0; node_id < graph.number_of_nodes(); ++node_id) {
        if (has_parent(node_id)) {
            NodeId parent_node = get_parent(node_id);
            cout << node_id << " -> " << parent_node << "\n";
        } else {
            cout << node_id << " -> None\n";
        }
    }
}
