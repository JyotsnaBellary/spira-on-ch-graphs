#pragma once
#include <iostream>
#include <enums.hpp>
#include <benchmark-tests.hpp>
#include <shortest-path/bi-directional/bi-directional-dijkstra.hpp>
// #include <shortest-path/bi-directional/bi-directional-spira.hpp>
#include <shortest-path/dijkstra.hpp>
#include <shortest-path/spira.hpp>
#include <types.hpp>

using namespace std;
class BenchmarkEliminationTree: public BenchmarkTests {
    public: 
        static void run_src_dst_benchmark_with_elimination_tree(const CH_Graph &graph, const string &output_csv_path);
        static void run_dijkstra_with_elimination_tree(NodeId src, NodeId dst);
        static void run_dijkstra_with_priority_queue();
};