#pragma once

#include <string>
#include <vector>

struct FilterSpec {
    std::string name;
    std::vector<std::string> args;
};

struct ParsedCommand {
    std::string input_path;
    std::string output_path;
    std::vector<FilterSpec> filters;
};

class Parser {
public:
    ParsedCommand Parse(int argc, const char* argv[]) const;
};
