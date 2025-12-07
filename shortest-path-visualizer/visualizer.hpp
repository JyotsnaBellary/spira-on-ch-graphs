#pragma once
#include <string>
#include <file_handler.hpp>
#include <dijkstra.hpp>
#include <spira.hpp>
#include <new_variant.hpp>
#include <chrono>

using namespace std;
using Clock = chrono::high_resolution_clock;

namespace WebAPP {

    // Main unified visualize entry
    void visualize_algorithm(const string& filename,
                             const string& algorithm = "Dijkstra",
                             int src = 0,
                             int trg = -1,
                             const WeightMode& weightMode = WeightMode::Original,
                             const string& graphType = "default");

    // Algorithm-specific visualize
    void visualize_dijkstra(const string& filename,
                            int src,
                            int trg,
                            const WeightMode& weightMode,
                            const string& algorithm);

    void visualize_spira(const string& filename,
                         int src,
                         int trg,
                         const WeightMode& weightMode,
                         const string& algorithm);

    void visualize_newvariant(const string& filename,
                              int src,
                              int trg,
                              const WeightMode& weightMode,
                              const string& algorithm);

}
