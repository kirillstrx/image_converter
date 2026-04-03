#pragma once

#include <string>

#include "image.h"

class PngReader {
public:
    Image Read(const std::string& path) const;
};

class PngWriter {
public:
    void Write(const std::string& path, const Image& image) const;
};