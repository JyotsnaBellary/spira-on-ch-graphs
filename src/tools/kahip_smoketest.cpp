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

// static std::vector<int> read_partition_labels(const std::filesystem::path& file) {
//     std::ifstream in(file);
//     if (!in) throw std::runtime_error("Cannot open output file: " + file.string());
//     std::vector<int> labels;
//     int v;
//     while (in >> v) labels.push_back(v);
//     return labels;
// }

// int main(int argc, char** argv) {
// #ifndef KAHIP_NODE_SEPARATOR_PATH
// #error "KAHIP_NODE_SEPARATOR_PATH must be defined by CMake"
// #endif
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " /path/to/graph\n";
//         return 2;
//     }
//     const std::filesystem::path graph = argv[1];
//     if (!std::filesystem::exists(graph)) {
//         std::cerr << "Graph file not found: " << graph << "\n";
//         return 2;
//     }

//     // Use a unique temp directory so KaHIP writes its tmpseparator* there.
//     const auto tmp_root = std::filesystem::temp_directory_path();
//     const auto workdir  = tmp_root / std::filesystem::path("kahip-XXXXXX");
//     std::filesystem::create_directories(workdir);
//     const auto old_cwd = std::filesystem::current_path();
//     std::filesystem::current_path(workdir);

//     // Build command: quote paths; capture stderr too.
//     const std::string kahip = KAHIP_NODE_SEPARATOR_PATH; // provided by CMake
//     const std::string cmd   = "\"" + kahip + "\" \"" + graph.string() + "\" 2>&1";

//     std::cout << "[*] Running: " << cmd << "\n";
//     std::array<char, 4096> buf{};
//     std::string out;
//     FILE* pipe = POPEN(cmd.c_str(), "r");
//     if (!pipe) {
//         std::cerr << "Failed to spawn KaHIP.\n";
//         return 3;
//     }
//     while (fgets(buf.data(), (int)buf.size(), pipe)) out.append(buf.data());
//     int status = PCLOSE(pipe);

// #if !defined(_WIN32)
//     int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : status;
// #else
//     int exit_code = status;
// #endif

//     std::filesystem::current_path(old_cwd);

//     std::cout << "----- KaHIP OUTPUT BEGIN -----\n" << out
//               << "----- KaHIP OUTPUT END -----\n";
//     std::cout << "[*] Exit code: " << exit_code << "\n";
//     if (exit_code != 0) {
//         std::cerr << "KaHIP returned non-zero exit code.\n";
//         return exit_code;
//     }

//     // Find the output file name (e.g., "tmpseparator2") in the log.
//     std::smatch m;
//     std::regex rx_out(R"(writing\s+partition\s+to\s+(\S+))");
//     std::string out_file_name;
//     if (std::regex_search(out, m, rx_out)) out_file_name = m[1];

//     std::filesystem::path out_path = workdir / (out_file_name.empty() ? "tmpseparator2" : out_file_name);
//     if (!std::filesystem::exists(out_path)) {
//         std::cerr << "Expected output file not found: " << out_path << "\n";
//         return 4;
//     }

//     auto labels = read_partition_labels(out_path);
//     std::cout << "[*] Output file: " << out_path << "\n";
//     std::cout << "[*] Labels read: " << labels.size() << "\n";

//     // Print a quick peek at the first 10 labels
//     std::cout << "[*] First labels: ";
//     for (size_t i = 0; i < std::min<size_t>(10, labels.size()); ++i) {
//         std::cout << labels[i] << (i + 1 < std::min<size_t>(10, labels.size()) ? ' ' : '\n');
//     }

//     // (Optional) Count how many nodes belong to the "separator" block.
//     // Often for 2-way + separator, label 2 is the separator. Adjust as needed.
//     size_t sep_count = 0;
//     for (int v : labels) if (v == 2) ++sep_count;
//     std::cout << "[*] Heuristic separator label==2 count: " << sep_count << "\n";

//     return 0;
// }
