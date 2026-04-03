#pragma once

#include <memory>
#include <vector>

#include "filters.h"

class Pipeline {
public:
    void Add(std::unique_ptr<Filter> filter);
    void Apply(Image& image) const;

private:
    std::vector<std::unique_ptr<Filter>> filters_;
};
