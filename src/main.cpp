#include <iostream>
#include "file_handler.hpp"
#include <dijkstra.hpp>
#include <spira.hpp>
#include <new_variant.hpp>
#include <chrono>
#include <set>
#include <random>
#include <basic_test.cpp>
#include <benchmark_tests.cpp>

using namespace std;



int main()
{
    // run_benchmark_on_exponential_size_sweep();
    run_benchmark_on_sparse_graphs();
    run_benchmark_on_dense_graphs();
    // run_all_algorithms_on_exponential_weights();
}