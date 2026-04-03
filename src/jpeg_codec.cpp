#include "jpeg_codec.h"

#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <jpeglib.h>
}

namespace {

constexpr std::uint32_t MAX_COLOR_COMPONENT = 255;
constexpr int JPEG_QUALITY = 95;

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

struct JpegErrorManager {
    jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
    char message[JMSG_LENGTH_MAX];
};

void OnJpegError(j_common_ptr cinfo) {
    JpegErrorManager* error_manager = reinterpret_cast<JpegErrorManager*>(cinfo->err);
    (*cinfo->err->format_message)(cinfo, error_manager->message);
    longjmp(error_manager->setjmp_buffer, 1);
}

}  // namespace

Image JpegReader::Read(const std::string& path) const {
    FILE* input = std::fopen(path.c_str(), "rb");
    if (input == nullptr) {
        throw std::runtime_error("Cannot open JPEG file: " + path);
    }

    jpeg_decompress_struct cinfo{};
    JpegErrorManager jerr{};

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = OnJpegError;

    if (setjmp(jerr.setjmp_buffer) != 0) {
        jpeg_destroy_decompress(&cinfo);
        std::fclose(input);
        throw std::runtime_error(std::string("JPEG read error: ") + jerr.message);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, input);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB;

    jpeg_start_decompress(&cinfo);

    int width = static_cast<int>(cinfo.output_width);
    int height = static_cast<int>(cinfo.output_height);
    int channels = static_cast<int>(cinfo.output_components);

    if (channels != 3) {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        std::fclose(input);
        throw std::runtime_error("Unsupported JPEG channel count");
    }

    Image image(width, height);
    std::vector<JSAMPLE> row_buffer(static_cast<std::size_t>(width) * channels);

    int y = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW row_pointer = row_buffer.data();
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);

        for (int x = 0; x < width; ++x) {
            std::size_t index = static_cast<std::size_t>(x) * 3;
            image.At(x, y) = Color(
                ToDouble(row_buffer[index]),
                ToDouble(row_buffer[index + 1]),
                ToDouble(row_buffer[index + 2])
            );
        }
        ++y;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    std::fclose(input);

    return image;
}

void JpegWriter::Write(const std::string& path, const Image& image) const {
    if (image.IsEmpty()) {
        throw std::runtime_error("Cannot write empty image to JPEG file: " + path);
    }

    FILE* output = std::fopen(path.c_str(), "wb");
    if (output == nullptr) {
        throw std::runtime_error("Cannot open output JPEG file: " + path);
    }

    jpeg_compress_struct cinfo{};
    JpegErrorManager jerr{};

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = OnJpegError;

    if (setjmp(jerr.setjmp_buffer) != 0) {
        jpeg_destroy_compress(&cinfo);
        std::fclose(output);
        throw std::runtime_error(std::string("JPEG write error: ") + jerr.message);
    }

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, output);

    cinfo.image_width = static_cast<JDIMENSION>(image.GetWidth());
    cinfo.image_height = static_cast<JDIMENSION>(image.GetHeight());
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    std::vector<JSAMPLE> row_buffer(static_cast<std::size_t>(image.GetWidth()) * 3);

    while (cinfo.next_scanline < cinfo.image_height) {
        int y = static_cast<int>(cinfo.next_scanline);

        for (int x = 0; x < image.GetWidth(); ++x) {
            std::size_t index = static_cast<std::size_t>(x) * 3;
            const Color& color = image.At(x, y);

            row_buffer[index] = ToByte(color.GetRed());
            row_buffer[index + 1] = ToByte(color.GetGreen());
            row_buffer[index + 2] = ToByte(color.GetBlue());
        }

        JSAMPROW row_pointer = row_buffer.data();
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    std::fclose(output);
}