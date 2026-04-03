#include "parser.h"

#include <sstream>
#include <stdexcept>

namespace {

bool IsFilterName(const std::string& token) {
    return !token.empty() && token[0] == '-';
}

}  // namespace

ParsedCommand Parser::Parse(const int argc, const char* argv[]) const {
    if (argc < 3) {
        throw std::invalid_argument("Expected input and output file paths");
    }

    ParsedCommand command;
    command.input_path = argv[1];
    command.output_path = argv[2];

    int index = 3;
    while (index < argc) {
        std::string token = argv[index];
        if (!IsFilterName(token)) {
            throw std::invalid_argument("Expected filter name starting with '-' near argument: " + token);
        }

        FilterSpec spec;
        spec.name = token.substr(1);
        if (spec.name.empty()) {
            throw std::invalid_argument("Filter name cannot be empty");
        }

        ++index;
        while (index < argc && !IsFilterName(argv[index])) {
            spec.args.push_back(argv[index]);
            ++index;
        }

        command.filters.push_back(spec);
    }

    return command;
}
