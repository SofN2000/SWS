#include "file_to_string.hh"

#include <fstream>
#include <iostream>
#include <sstream>

std::string file_to_string(const std::string &path)
{
    auto file_in = std::ifstream(path);
    if (!file_in.is_open())
        throw std::exception{};
    auto sstream = std::stringstream{};
    sstream << file_in.rdbuf();
    return sstream.str();
}
