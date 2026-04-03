#include "pipeline.h"

#include <utility>

void Pipeline::Add(std::unique_ptr<Filter> filter) {
    filters_.push_back(std::move(filter));
}

void Pipeline::Apply(Image& image) const {
    for (const auto& filter : filters_) {
        filter->Apply(image);
    }
}
