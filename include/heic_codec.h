#pragma once

#include <string>

#include "image.h"

class HeicReader {
public:
    Image Read(const std::string& path) const;
};

class HeicWriter {
public:
    void Write(const std::string& path, const Image& image) const;
};