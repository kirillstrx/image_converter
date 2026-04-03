#pragma once

#include <string>

#include "image.h"

class BmpReader {
public:
    Image Read(const std::string& path) const;
};

class BmpWriter {
public:
    void Write(const std::string& path, const Image& image) const;
};
