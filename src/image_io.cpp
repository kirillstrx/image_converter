#include "image_io.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

#include "bmp.h"
#include "png_codec.h"
#include "jpeg_codec.h"
#include "heic_codec.h"

namespace {

    std::string ToLower(std::string s) {
        for (char& c : s) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return s;
    }

    std::string GetExtension(const std::string& path) {
        const std::size_t pos = path.find_last_of('.');
        if (pos == std::string::npos) {
            return "";
        }
        return ToLower(path.substr(pos));
    }

}  // namespace

Image ReadImage(const std::string& path) {
    const std::string ext = GetExtension(path);

    if (ext == ".bmp") {
        return BmpReader().Read(path);
    }
    if (ext == ".png") {
        return PngReader().Read(path);
    }
    if (ext == ".jpg" || ext == ".jpeg") {
        return JpegReader().Read(path);
    }
    if (ext == ".heic" || ext == ".heif") {
        return HeicReader().Read(path);
    }

    throw std::runtime_error("Unsupported input format: " + path);
}

void WriteImage(const std::string& path, const Image& image) {
    const std::string ext = GetExtension(path);

    if (ext == ".bmp") {
        BmpWriter().Write(path, image);
        return;
    }
    if (ext == ".png") {
        PngWriter().Write(path, image);
        return;
    }
    if (ext == ".jpg" || ext == ".jpeg") {
        JpegWriter().Write(path, image);
        return;
    }
    if (ext == ".heic" || ext == ".heif") {
        HeicWriter().Write(path, image);
        return;
    }

    throw std::runtime_error("Unsupported output format: " + path);
}