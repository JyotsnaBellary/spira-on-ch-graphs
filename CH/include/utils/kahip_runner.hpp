#pragma once
#include <filesystem>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

filesystem::path run_kahip_on_test_graph(const char* out_name = "nodeOrder");
