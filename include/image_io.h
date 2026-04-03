#pragma once

#include <string>

#include "image.h"

Image ReadImage(const std::string& path);
void WriteImage(const std::string& path, const Image& image);