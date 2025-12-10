#pragma once

#include <string>
#include <graph.hpp>
#include <sssp_result.hpp>
#include <benchmark_tests.hpp>
#include <visualizer.hpp>

// Run the interactive menu
void run_interactive_cli();

// Helper functions
void print_algorithm_summary(const string& name,
                             const SsspResult& result);

void print_pertinence_summary(const PertinenceStats& stats);

void compare_paths(const SsspResult& reference_result,
                   const SsspResult& test_result);
