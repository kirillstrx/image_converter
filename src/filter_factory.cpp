#include "filter_factory.h"

#include <stdexcept>
#include <string>

namespace {

int ParseInt(const std::string& value, const std::string& parameter_name) {
    std::size_t position = 0;
    const int result = std::stoi(value, &position);
    if (position != value.size()) {
        throw std::invalid_argument("Invalid integer value for " + parameter_name + ": " + value);
    }
    return result;
}

double ParseDouble(const std::string& value, const std::string& parameter_name) {
    std::size_t position = 0;
    const double result = std::stod(value, &position);
    if (position != value.size()) {
        throw std::invalid_argument("Invalid floating-point value for " + parameter_name + ": " + value);
    }
    return result;
}

void RequireArgumentCount(const FilterSpec& spec, const std::size_t expected) {
    if (spec.args.size() != expected) {
        throw std::invalid_argument("Filter -" + spec.name + " expects " + std::to_string(expected) +
                                    " argument(s), got " + std::to_string(spec.args.size()));
    }
}

}  // namespace

std::unique_ptr<Filter> FilterFactory::Create(const FilterSpec& spec) {
    if (spec.name == "crop") {
        RequireArgumentCount(spec, 2);
        return std::make_unique<CropFilter>(ParseInt(spec.args[0], "crop width"),
                                            ParseInt(spec.args[1], "crop height"));
    }
    if (spec.name == "gs") {
        RequireArgumentCount(spec, 0);
        return std::make_unique<GrayscaleFilter>();
    }
    if (spec.name == "neg") {
        RequireArgumentCount(spec, 0);
        return std::make_unique<NegativeFilter>();
    }
    if (spec.name == "sharp") {
        RequireArgumentCount(spec, 0);
        return std::make_unique<SharpeningFilter>();
    }
    if (spec.name == "edge") {
        RequireArgumentCount(spec, 1);
        return std::make_unique<EdgeDetectionFilter>(ParseDouble(spec.args[0], "edge threshold"));
    }
    if (spec.name == "blur") {
        RequireArgumentCount(spec, 1);
        return std::make_unique<GaussianBlurFilter>(ParseDouble(spec.args[0], "blur sigma"));
    }
    if (spec.name == "glow") {
        RequireArgumentCount(spec, 3);
        return std::make_unique<GlowFilter>(ParseDouble(spec.args[0], "glow threshold"),
                                            ParseInt(spec.args[1], "glow radius"),
                                            ParseDouble(spec.args[2], "glow intensity"));
    }
    if (spec.name == "contrast") {
        RequireArgumentCount(spec, 1);
        return std::make_unique<ContrastFilter>(ParseDouble(spec.args[0], "coefficient"));
    }
    throw std::invalid_argument("Unknown filter: -" + spec.name);
}
