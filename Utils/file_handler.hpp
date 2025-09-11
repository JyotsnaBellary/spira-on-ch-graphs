#pragma once
#include <graph.hpp>
#include <string>

class FileHandler {
    public:
        // Reads graph Input
        Graph read_file(const string& file_path);
};