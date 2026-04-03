#include "png_codec.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <png.h>

namespace {

constexpr std::uint32_t MAX_COLOR_COMPONENT = 255;

std::uint8_t ToByte(double value) {
    if (value < 0.0) {
        value = 0.0;
    }
    if (value > 1.0) {
        value = 1.0;
    }
    return static_cast<std::uint8_t>(value * MAX_COLOR_COMPONENT + 0.5);
}

double ToDouble(std::uint8_t value) {
    return static_cast<double>(value) / MAX_COLOR_COMPONENT;
}

void ThrowPngError(const png_image& image, const std::string& fallback) {
    if (image.message[0] != '\0') {
        throw std::runtime_error(std::string("PNG error: ") + image.message);
    }
    throw std::runtime_error(fallback);
}

}  // namespace

Image PngReader::Read(const std::string& path) const {
    png_image image_info{};
    image_info.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image_info, path.c_str())) {
        ThrowPngError(image_info, "Cannot open PNG file: " + path);
    }

    image_info.format = PNG_FORMAT_RGB;
    std::vector<png_byte> buffer(PNG_IMAGE_SIZE(image_info));

    if (!png_image_finish_read(&image_info, nullptr, buffer.data(), 0, nullptr)) {
        png_image_free(&image_info);
        ThrowPngError(image_info, "Failed to read PNG file: " + path);
    }

    Image image(static_cast<int>(image_info.width), static_cast<int>(image_info.height));

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            std::size_t index = static_cast<std::size_t>(y * image.GetWidth() + x) * 3;
            image.At(x, y) = Color(
                ToDouble(buffer[index]),
                ToDouble(buffer[index + 1]),
                ToDouble(buffer[index + 2])
            );
        }
    }

    png_image_free(&image_info);
    return image;
}

void PngWriter::Write(const std::string& path, const Image& image) const {
    if (image.IsEmpty()) {
        throw std::runtime_error("Cannot write empty image to PNG file: " + path);
    }

    png_image image_info{};
    image_info.version = PNG_IMAGE_VERSION;
    image_info.width = static_cast<png_uint_32>(image.GetWidth());
    image_info.height = static_cast<png_uint_32>(image.GetHeight());
    image_info.format = PNG_FORMAT_RGB;

    std::vector<png_byte> buffer(
        static_cast<std::size_t>(image.GetWidth()) *
        static_cast<std::size_t>(image.GetHeight()) * 3
    );

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            std::size_t index = static_cast<std::size_t>(y * image.GetWidth() + x) * 3;
            const Color& color = image.At(x, y);

            buffer[index] = ToByte(color.GetRed());
            buffer[index + 1] = ToByte(color.GetGreen());
            buffer[index + 2] = ToByte(color.GetBlue());
        }
    }

    if (!png_image_write_to_file(&image_info, path.c_str(), 0, buffer.data(), 0, nullptr)) {
        ThrowPngError(image_info, "Failed to write PNG file: " + path);
    }

    png_image_free(&image_info);
}