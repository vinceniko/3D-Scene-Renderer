#pragma once

#include <string>
#include <sstream>
#include <fstream>

std::string get_file_str(const std::string& fpath) {
    std::ifstream f;
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::stringstream stream;
    f.open(fpath);
    stream << f.rdbuf();
    f.close();
    return stream.str();
}