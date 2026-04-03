#include "pipeline_builder.h"

#include "filter_factory.h"

Pipeline PipelineBuilder::Build(const std::vector<FilterSpec>& filter_specs) {
    Pipeline pipeline;
    for (const FilterSpec& filter_spec : filter_specs) {
        pipeline.Add(FilterFactory::Create(filter_spec));
    }
    return pipeline;
}
