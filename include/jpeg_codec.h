#pragma once

#include <string>

#include "image.h"

class JpegReader {
public:
    Image Read(const std::string& path) const;
};

class JpegWriter {
public:
    void Write(const std::string& path, const Image& image) const;
};