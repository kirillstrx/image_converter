#pragma once

#include <vector>

#include "parser.h"
#include "pipeline.h"

class PipelineBuilder {
public:
    static Pipeline Build(const std::vector<FilterSpec>& filter_specs);
};
