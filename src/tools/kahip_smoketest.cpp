// #include <array>
// #include <cstdio>
// #include <cstdlib>
// #include <filesystem>
// #include <fstream>
// #include <iostream>
// #include <regex>
// #include <string>
// #include <vector>

// #if defined(_WIN32)
// #  define POPEN  _popen
// #  define PCLOSE _pclose
// #else
// #  define POPEN  popen
// #  define PCLOSE pclose
// #endif

// static vector<int> read_partition_labels(const filesystem::path& file) {
//     ifstream in(file);
//     if (!in) throw runtime_error("Cannot open output file: " + file.string());
//     vector<int> labels;
//     int v;
//     while (in >> v) labels.push_back(v);
//     return labels;
// }

// int main(int argc, char** argv) {
// #ifndef KAHIP_NODE_SEPARATOR_PATH
// #error "KAHIP_NODE_SEPARATOR_PATH must be defined by CMake"
// #endif
//     if (argc < 2) {
//         cerr << "Usage: " << argv[0] << " /path/to/graph\n";
//         return 2;
//     }
//     const filesystem::path graph = argv[1];
//     if (!filesystem::exists(graph)) {
//         cerr << "Graph file not found: " << graph << "\n";
//         return 2;
//     }

//     // Use a unique temp directory so KaHIP writes its tmpseparator* there.
//     const auto tmp_root = filesystem::temp_directory_path();
//     const auto workdir  = tmp_root / filesystem::path("kahip-XXXXXX");
//     filesystem::create_directories(workdir);
//     const auto old_cwd = filesystem::current_path();
//     filesystem::current_path(workdir);

//     // Build command: quote paths; capture stderr too.
//     const string kahip = KAHIP_NODE_SEPARATOR_PATH; // provided by CMake
//     const string cmd   = "\"" + kahip + "\" \"" + graph.string() + "\" 2>&1";

//     cout << "[*] Running: " << cmd << "\n";
//     array<char, 4096> buf{};
//     string out;
//     FILE* pipe = POPEN(cmd.c_str(), "r");
//     if (!pipe) {
//         cerr << "Failed to spawn KaHIP.\n";
//         return 3;
//     }
//     while (fgets(buf.data(), (int)buf.size(), pipe)) out.append(buf.data());
//     int status = PCLOSE(pipe);

// #if !defined(_WIN32)
//     int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : status;
// #else
//     int exit_code = status;
// #endif

//     filesystem::current_path(old_cwd);

//     cout << "----- KaHIP OUTPUT BEGIN -----\n" << out
//               << "----- KaHIP OUTPUT END -----\n";
//     cout << "[*] Exit code: " << exit_code << "\n";
//     if (exit_code != 0) {
//         cerr << "KaHIP returned non-zero exit code.\n";
//         return exit_code;
//     }

//     // Find the output file name (e.g., "tmpseparator2") in the log.
//     smatch m;
//     regex rx_out(R"(writing\s+partition\s+to\s+(\S+))");
//     string out_file_name;
//     if (regex_search(out, m, rx_out)) out_file_name = m[1];

//     filesystem::path out_path = workdir / (out_file_name.empty() ? "tmpseparator2" : out_file_name);
//     if (!filesystem::exists(out_path)) {
//         cerr << "Expected output file not found: " << out_path << "\n";
//         return 4;
//     }

//     auto labels = read_partition_labels(out_path);
//     cout << "[*] Output file: " << out_path << "\n";
//     cout << "[*] Labels read: " << labels.size() << "\n";

//     // Print a quick peek at the first 10 labels
//     cout << "[*] First labels: ";
//     for (size_t i = 0; i < min<size_t>(10, labels.size()); ++i) {
//         cout << labels[i] << (i + 1 < min<size_t>(10, labels.size()) ? ' ' : '\n');
//     }

//     // (Optional) Count how many nodes belong to the "separator" block.
//     // Often for 2-way + separator, label 2 is the separator. Adjust as needed.
//     size_t sep_count = 0;
//     for (int v : labels) if (v == 2) ++sep_count;
//     cout << "[*] Heuristic separator label==2 count: " << sep_count << "\n";

//     return 0;
// }
