#pragma once

#include <memory>

#include "filters.h"
#include "parser.h"

class FilterFactory {
public:
    static std::unique_ptr<Filter> Create(const FilterSpec& spec);
};
