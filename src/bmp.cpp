#include "bmp.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace {

constexpr std::uint16_t BmpSignature = 0x4D42;
constexpr std::uint32_t BmpInfoHeaderSize = 40;
constexpr std::uint16_t BmpBitsPerPixel = 24;
constexpr std::uint32_t BmpCompression = 0;
constexpr std::uint32_t BmpFileHeaderSize = 14;
constexpr std::uint32_t MaxColorComponent = 255;

#pragma pack(push, 1)
struct BmpFileHeader {
    std::uint16_t type;
    std::uint32_t size;
    std::uint16_t reserved1;
    std::uint16_t reserved2;
    std::uint32_t offset_bits;
};

struct BmpInfoHeader {
    std::uint32_t size;
    std::int32_t width;
    std::int32_t height;
    std::uint16_t planes;
    std::uint16_t bit_count;
    std::uint32_t compression;
    std::uint32_t size_image;
    std::int32_t x_pixels_per_meter;
    std::int32_t y_pixels_per_meter;
    std::uint32_t clr_used;
    std::uint32_t clr_important;
};
#pragma pack(pop)

static_assert(sizeof(BmpFileHeader) == BmpFileHeaderSize);
static_assert(sizeof(BmpInfoHeader) == BmpInfoHeaderSize);

int GetRowPadding(const int width) {
    const int row_size = width * 3;
    return (4 - row_size % 4) % 4;
}

std::uint8_t ToByte(const double value) {
    const double scaled = value * static_cast<double>(MaxColorComponent);
    const double rounded = scaled + 0.5;
    if (rounded <= 0.0) {
        return 0;
    }
    if (rounded >= static_cast<double>(MaxColorComponent)) {
        return static_cast<std::uint8_t>(MaxColorComponent);
    }
    return static_cast<std::uint8_t>(rounded);
}

double ToComponent(const std::uint8_t value) {
    return static_cast<double>(value) / static_cast<double>(MaxColorComponent);
}

void CheckStream(const std::ios& stream, const std::string& message) {
    if (!stream) {
        throw std::runtime_error(message);
    }
}

}  // namespace

Image BmpReader::Read(const std::string& path) const {
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        throw std::runtime_error("Cannot open input file: " + path);
    }

    BmpFileHeader file_header{};
    BmpInfoHeader info_header{};
    input.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    input.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    CheckStream(input, "Failed to read BMP headers from file: " + path);

    if (file_header.type != BmpSignature) {
        throw std::runtime_error("Input file is not a BMP file: " + path);
    }
    if (info_header.size != BmpInfoHeaderSize) {
        throw std::runtime_error("Unsupported DIB header size in file: " + path);
    }
    if (info_header.planes != 1) {
        throw std::runtime_error("Unsupported number of color planes in file: " + path);
    }
    if (info_header.bit_count != BmpBitsPerPixel) {
        throw std::runtime_error("Only 24-bit BMP files are supported: " + path);
    }
    if (info_header.compression != BmpCompression) {
        throw std::runtime_error("Compressed BMP files are not supported: " + path);
    }
    if (info_header.width <= 0 || info_header.height == 0) {
        throw std::runtime_error("Invalid BMP image dimensions in file: " + path);
    }
    if (file_header.offset_bits < BmpFileHeaderSize + BmpInfoHeaderSize) {
        throw std::runtime_error("Invalid BMP pixel data offset in file: " + path);
    }

    const bool is_bottom_up = info_header.height > 0;
    const int width = info_header.width;
    const int height = is_bottom_up ? info_header.height : -info_header.height;
    Image image(width, height);

    input.seekg(static_cast<std::streamoff>(file_header.offset_bits), std::ios::beg);
    CheckStream(input, "Failed to seek to BMP pixel data in file: " + path);

    const int padding = GetRowPadding(width);
    std::array<char, 3> bgr{};
    std::array<char, 3> padding_bytes{};

    for (int row = 0; row < height; ++row) {
        const int y = is_bottom_up ? height - 1 - row : row;
        for (int x = 0; x < width; ++x) {
            input.read(bgr.data(), static_cast<std::streamsize>(bgr.size()));
            CheckStream(input, "Unexpected end of BMP pixel data in file: " + path);

            const std::uint8_t blue = static_cast<std::uint8_t>(static_cast<unsigned char>(bgr[0]));
            const std::uint8_t green = static_cast<std::uint8_t>(static_cast<unsigned char>(bgr[1]));
            const std::uint8_t red = static_cast<std::uint8_t>(static_cast<unsigned char>(bgr[2]));
            image.At(x, y) = Color(ToComponent(red), ToComponent(green), ToComponent(blue));
        }

        input.read(padding_bytes.data(), padding);
        CheckStream(input, "Unexpected end of BMP row padding in file: " + path);
    }

    return image;
}

void BmpWriter::Write(const std::string& path, const Image& image) const {
    if (image.GetWidth() <= 0 || image.GetHeight() <= 0) {
        throw std::runtime_error("Cannot write empty image to file: " + path);
    }

    std::ofstream output(path, std::ios::binary);
    if (!output.is_open()) {
        throw std::runtime_error("Cannot open output file: " + path);
    }

    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const int padding = GetRowPadding(width);
    const std::uint32_t row_size = static_cast<std::uint32_t>(width * 3 + padding);
    const std::uint32_t image_size = row_size * static_cast<std::uint32_t>(height);

    BmpFileHeader file_header{};
    file_header.type = BmpSignature;
    file_header.size = BmpFileHeaderSize + BmpInfoHeaderSize + image_size;
    file_header.offset_bits = BmpFileHeaderSize + BmpInfoHeaderSize;

    BmpInfoHeader info_header{};
    info_header.size = BmpInfoHeaderSize;
    info_header.width = width;
    info_header.height = height;
    info_header.planes = 1;
    info_header.bit_count = BmpBitsPerPixel;
    info_header.compression = BmpCompression;
    info_header.size_image = image_size;

    output.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    output.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    CheckStream(output, "Failed to write BMP headers to file: " + path);

    constexpr std::array<char, 3> PaddingBytes = {0, 0, 0};
    for (int row = height - 1; row >= 0; --row) {
        for (int x = 0; x < width; ++x) {
            const Color& color = image.At(x, row);
            const std::array<char, 3> bgr = {static_cast<char>(ToByte(color.GetBlue())),
                                             static_cast<char>(ToByte(color.GetGreen())),
                                             static_cast<char>(ToByte(color.GetRed()))};
            output.write(bgr.data(), static_cast<std::streamsize>(bgr.size()));
        }
        output.write(PaddingBytes.data(), padding);
        CheckStream(output, "Failed to write BMP pixel data to file: " + path);
    }
}
