#include "image.h"

#include <algorithm>
#include <stdexcept>

namespace {

double ClampComponent(const double value) {
    return std::clamp(value, 0.0, 1.0);
}

}  // namespace

Color::Color() : red_(0.0), green_(0.0), blue_(0.0) {
}

Color::Color(const double red, const double green, const double blue)
    : red_(ClampComponent(red)), green_(ClampComponent(green)), blue_(ClampComponent(blue)) {
}

double Color::GetRed() const {
    return red_;
}

double Color::GetGreen() const {
    return green_;
}

double Color::GetBlue() const {
    return blue_;
}

void Color::SetRed(const double red) {
    red_ = ClampComponent(red);
}

void Color::SetGreen(const double green) {
    green_ = ClampComponent(green);
}

void Color::SetBlue(const double blue) {
    blue_ = ClampComponent(blue);
}

void Color::Clamp() {
    red_ = ClampComponent(red_);
    green_ = ClampComponent(green_);
    blue_ = ClampComponent(blue_);
}

Image::Image() : width_(0), height_(0) {
}

Image::Image(const int width, const int height) : width_(width), height_(height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument("Image size must be non-negative");
    }
    pixels_ = std::vector<Color>(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_));
}

int Image::GetWidth() const {
    return width_;
}

int Image::GetHeight() const {
    return height_;
}

bool Image::IsEmpty() const {
    return width_ == 0 || height_ == 0;
}

bool Image::IsInside(const int x, const int y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

Color& Image::At(const int x, const int y) {
    return pixels_.at(GetIndex(x, y));
}

const Color& Image::At(const int x, const int y) const {
    return pixels_.at(GetIndex(x, y));
}

const Color& Image::GetClamped(int x, int y) const {
    if (IsEmpty()) {
        throw std::out_of_range("Cannot access pixel in empty image");
    }
    x = std::clamp(x, 0, width_ - 1);
    y = std::clamp(y, 0, height_ - 1);
    return At(x, y);
}

std::size_t Image::GetIndex(const int x, const int y) const {
    if (!IsInside(x, y)) {
        throw std::out_of_range("Pixel coordinates are out of range");
    }
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
}
