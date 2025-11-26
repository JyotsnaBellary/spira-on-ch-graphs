#pragma once
#include "graph.hpp"
#include "dijkstra.hpp"
#include <queue>

class NewVariant
{
    private:
        Graph& graph;

        // Initialise the threshold 
        // int median_distance;
        Cost median_distance;

        // adjacency pointers
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


        // set if Out[u] may still contain unscanned out-pertinent edges
        vector<bool> can_scan_out;

        // 
    public:
        NewVariant(Graph& graph);
        

        bool is_pertinent(EdgeId edgeId) const;
        bool is_in_pertinent(EdgeId edgeId) const;
        bool is_out_pertinent(EdgeId edgeId) const;

        void forward(NodeId nodeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);
        void backward(NodeId nodeId, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);

        void append_to_request(NodeId nodeId, EdgeId edgeId, vector<Cost>& cost, priority_queue<pair<Cost, EdgeId>, vector<pair<Cost, EdgeId>>, greater<pair<Cost, EdgeId>>>& pq);

        void reset(vector<int> &next_index);
        EdgeId next(vector<int> &next_index, NodeId &nodeId, const vector<EdgeId>& adjacent_neighbor_edges);

        const vector<EdgeId>& get_requested_neighbors(NodeId nodeId) const;
        
        //Computes shortest path between src and dst
        SsspResult compute_shortest_path(NodeId src, NodeId dst);
        
        //builds result path
        SsspResult build_path(const vector<int>& prev, const vector<Cost>& cost, const vector<EdgeId>& viaEdge, NodeId dst, int number_of_pops, int number_of_Q_pops, Cost median);
};

