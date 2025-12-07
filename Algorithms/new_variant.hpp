#pragma once
#include "graph.hpp"
#include "sssp_result.hpp"
#include <queue>

class NewVariant
{
    private:
        Graph& graph;

        // The threshold M
        Cost median_distance;

        // Adjacency pointers
        vector<int> next_index_out_adj;
        vector<int> next_index_in_adj;
        vector<int> next_index_request;

        // vector<char> is_sorted;
        vector<bool> in_pertinent_edges;
        vector<bool> out_pertinent_edges;
        vector<bool> in_pertinent_edges_extracted_in_forward_phase;

        // Req[u] contains in-pertinent edges whose scan was specifically requested
        vector<vector<EdgeId>> req;
        vector<bool> active;
        vector<bool> in_S;

        // Out[u] in paper. Keeps track of what nodes can still be scanned out in adj list
        vector<bool> can_scan_out;

    public:
        NewVariant(Graph& graph);
        
        // Checks if an edge is pertinent
        bool is_pertinent(EdgeId edgeId) const;
        bool is_in_pertinent(EdgeId edgeId) const;
        bool is_out_pertinent(EdgeId edgeId) const;

        // Forward scan to insert next outgoing edge of a vertex u into P
        void forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);

        // Backward scan to insert next incoming edge of a vertex v into Q
        void backward(NodeId nodeId, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);

        // Inserts a requested edge to Req. Calls Forward scan on edge if urgent
        void append_to_request(NodeId nodeId, EdgeId edgeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);

        // resets any next_index pointer to point to the start of the list 
        void reset(vector<int> &next_index);

        // Returns the next edge from a list of edges
        EdgeId next(vector<int> &next_index, NodeId &nodeId, const vector<EdgeId>& adjacent_neighbor_edges);

        // Returns requested in-pertinent edges from Req[]
        const vector<EdgeId>& get_requested_neighbors(NodeId nodeId) const;
        
        //Computes shortest path between src and dst
        SsspResult compute_shortest_path(NodeId src, NodeId dst);
        
        //builds result path
        SsspResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops, int number_of_Q_pops, Cost median);
};
