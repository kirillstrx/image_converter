#include "processor.h"

#include "image_io.h"
#include "pipeline_builder.h"

void Processor::Run(const ParsedCommand& command) const {
    Image image = ReadImage(command.input_path);
    const Pipeline pipeline = PipelineBuilder::Build(command.filters);
    pipeline.Apply(image);
    WriteImage(command.output_path, image);
}