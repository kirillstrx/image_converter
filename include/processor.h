#pragma once

#include "parser.h"

class Processor {
public:
    void Run(const ParsedCommand& command) const;
};