#include <array>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#  define POPEN  _popen
#  define PCLOSE _pclose
#else
#  define POPEN  popen
#  define PCLOSE pclose
#endif

using namespace std;

filesystem::path run_kahip_on_test_graph(const char* out_name) {
#ifndef KAHIP_NODE_ORDERING_PATH
# error "KAHIP_NODE_ORDERING_PATH must be defined by CMake"
#endif
#ifndef ERP_SOURCE_DIR
# error "ERP_SOURCE_DIR must be defined by CMake"
#endif
    namespace fs = filesystem;

    // input is *not* under src, so keep your adjusted path:
    const fs::path graph   = fs::path(ERP_SOURCE_DIR) / "Input_Data/test.graph";
    const fs::path out_dir = fs::path(ERP_SOURCE_DIR) / "output_files/nested_dissections";

    if (!fs::exists(graph)) {
        throw runtime_error("Graph not found: " + graph.string());
    }
    fs::create_directories(out_dir);

    // Run KaHIP inside the output directory so it writes here
    const fs::path old_cwd = fs::current_path();
    fs::current_path(out_dir);

    const string bin = KAHIP_NODE_ORDERING_PATH;
    const string cmd = "\"" + bin + "\" \"" + graph.string() +
                            "\" --output_filename=" + out_name + " 2>&1";

    array<char, 4096> buf{};
    string log;
    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe) {
        fs::current_path(old_cwd);
        throw runtime_error("Failed to start KaHIP (node_ordering)");
    }
    while (fgets(buf.data(), (int)buf.size(), pipe)) log.append(buf.data());
    int status = PCLOSE(pipe);

#if !defined(_WIN32)
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : status;
#else
    int exit_code = status;
#endif
    fs::current_path(old_cwd);

    if (exit_code != 0) {
        throw runtime_error("KaHIP failed (exit " + to_string(exit_code) + ")\n" + log);
    }

    // --- Parse actual filename from KaHIP log (handles "level12", "tmpseparator2", etc.)
    smatch m;
    regex rx(R"(writing\s+partition\s+to\s+(\S+))");
    if (regex_search(log, m, rx)) {
        fs::path actual = out_dir / m[1].str();  // e.g., level12
        if (fs::exists(actual)) return actual;
    }

    // --- Fallbacks: try base and base+"2"
    fs::path cand1 = out_dir / out_name;                    // e.g., level1
    fs::path cand2 = out_dir / (string(out_name) + "2"); // e.g., level12
    if (fs::exists(cand1)) return cand1;
    if (fs::exists(cand2)) return cand2;

    // --- Debug: include directory listing + KaHIP log to see what's going on
    string listing;
    for (auto& p : fs::directory_iterator(out_dir)) {
        listing += p.path().filename().string() + "\n";
    }
    throw runtime_error(
        "Expected output missing in: " + out_dir.string() +
        "\nTried: " + cand1.string() + " and " + cand2.string() +
        "\nKaHIP log:\n" + log +
        "\nDirectory contents:\n" + listing
    );
}
