#include "heic_codec.h"

#include <cstdint>
#include <stdexcept>
#include <string>

#include <libheif/heif.h>

namespace {

constexpr std::uint32_t MAX_COLOR_COMPONENT = 255;
constexpr int HEIC_QUALITY = 90;

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

void CheckHeifError(const heif_error& error, const std::string& message) {
    if (error.code != heif_error_Ok) {
        if (error.message != nullptr && error.message[0] != '\0') {
            throw std::runtime_error(message + ": " + error.message);
        }
        throw std::runtime_error(message);
    }
}

}  // namespace

Image HeicReader::Read(const std::string& path) const {
    heif_context* context = heif_context_alloc();
    if (context == nullptr) {
        throw std::runtime_error("Failed to allocate HEIC context");
    }

    heif_image_handle* handle = nullptr;
    heif_image* decoded_image = nullptr;

    try {
        CheckHeifError(
            heif_context_read_from_file(context, path.c_str(), nullptr),
            "Cannot read HEIC file"
        );

        CheckHeifError(
            heif_context_get_primary_image_handle(context, &handle),
            "Cannot get HEIC image handle"
        );

        CheckHeifError(
            heif_decode_image(handle, &decoded_image, heif_colorspace_RGB,
                              heif_chroma_interleaved_RGB, nullptr),
            "Cannot decode HEIC image"
        );

        int stride = 0;
        const std::uint8_t* data =
            heif_image_get_plane_readonly(decoded_image, heif_channel_interleaved, &stride);

        if (data == nullptr) {
            throw std::runtime_error("Failed to access HEIC pixel data");
        }

        int width = heif_image_handle_get_width(handle);
        int height = heif_image_handle_get_height(handle);

        Image image(width, height);

        for (int y = 0; y < height; ++y) {
            const std::uint8_t* row = data + static_cast<std::size_t>(y) * stride;
            for (int x = 0; x < width; ++x) {
                std::size_t index = static_cast<std::size_t>(x) * 3;
                image.At(x, y) = Color(
                    ToDouble(row[index]),
                    ToDouble(row[index + 1]),
                    ToDouble(row[index + 2])
                );
            }
        }

        heif_image_release(decoded_image);
        heif_image_handle_release(handle);
        heif_context_free(context);

        return image;
    } catch (...) {
        if (decoded_image != nullptr) {
            heif_image_release(decoded_image);
        }
        if (handle != nullptr) {
            heif_image_handle_release(handle);
        }
        heif_context_free(context);
        throw;
    }
}

void HeicWriter::Write(const std::string& path, const Image& image) const {
    if (image.IsEmpty()) {
        throw std::runtime_error("Cannot write empty image to HEIC file: " + path);
    }

    heif_context* context = heif_context_alloc();
    if (context == nullptr) {
        throw std::runtime_error("Failed to allocate HEIC context");
    }

    heif_image* heif_image_ptr = nullptr;
    heif_encoder* encoder = nullptr;

    try {
        CheckHeifError(
            heif_image_create(image.GetWidth(), image.GetHeight(),
                              heif_colorspace_RGB, heif_chroma_interleaved_RGB,
                              &heif_image_ptr),
            "Cannot create HEIC image"
        );

        CheckHeifError(
            heif_image_add_plane(heif_image_ptr, heif_channel_interleaved,
                                 image.GetWidth(), image.GetHeight(), 8),
            "Cannot allocate HEIC plane"
        );

        int stride = 0;
        std::uint8_t* data =
            heif_image_get_plane(heif_image_ptr, heif_channel_interleaved, &stride);

        if (data == nullptr) {
            throw std::runtime_error("Failed to get writable HEIC plane");
        }

        for (int y = 0; y < image.GetHeight(); ++y) {
            std::uint8_t* row = data + static_cast<std::size_t>(y) * stride;
            for (int x = 0; x < image.GetWidth(); ++x) {
                std::size_t index = static_cast<std::size_t>(x) * 3;
                const Color& color = image.At(x, y);

                row[index] = ToByte(color.GetRed());
                row[index + 1] = ToByte(color.GetGreen());
                row[index + 2] = ToByte(color.GetBlue());
            }
        }

        CheckHeifError(
            heif_context_get_encoder_for_format(context, heif_compression_HEVC, &encoder),
            "Cannot get HEIC encoder"
        );

        heif_encoder_set_lossy_quality(encoder, HEIC_QUALITY);

        CheckHeifError(
            heif_context_encode_image(context, heif_image_ptr, encoder, nullptr, nullptr),
            "Cannot encode HEIC image"
        );

        CheckHeifError(
            heif_context_write_to_file(context, path.c_str()),
            "Cannot write HEIC file"
        );

        if (encoder != nullptr) {
            heif_encoder_release(encoder);
        }
        if (heif_image_ptr != nullptr) {
            heif_image_release(heif_image_ptr);
        }
        heif_context_free(context);
    } catch (...) {
        if (encoder != nullptr) {
            heif_encoder_release(encoder);
        }
        if (heif_image_ptr != nullptr) {
            heif_image_release(heif_image_ptr);
        }
        heif_context_free(context);
        throw;
    }
}