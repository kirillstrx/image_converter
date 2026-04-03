#include "filters.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {

constexpr double GrayscaleRed = 0.299;
constexpr double GrayscaleGreen = 0.587;
constexpr double GrayscaleBlue = 0.114;
constexpr double PI = 3.14159265358979323846;
constexpr double SharpenCenterWeight = 5.0;
constexpr double EdgeCenterWeight = 4.0;
double GetLuminance(const Color& color) {
    return GrayscaleRed * color.GetRed() + GrayscaleGreen * color.GetGreen() + GrayscaleBlue * color.GetBlue();
}

}  // namespace

CropFilter::CropFilter(const int width, const int height) : width_(width), height_(height) {
    if (width_ <= 0 || height_ <= 0) {
        throw std::invalid_argument("Crop size must be positive");
    }
}

void CropFilter::Apply(Image& image) const {
    const int new_width = std::min(width_, image.GetWidth());
    const int new_height = std::min(height_, image.GetHeight());
    Image cropped(new_width, new_height);
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            cropped.At(x, y) = image.At(x, y);
        }
    }
    image = std::move(cropped);
}

void GrayscaleFilter::Apply(Image& image) const {
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            Color& color = image.At(x, y);
            const double gray =
                GrayscaleRed * color.GetRed() + GrayscaleGreen * color.GetGreen() + GrayscaleBlue * color.GetBlue();
            color = Color(gray, gray, gray);
        }
    }
}

void NegativeFilter::Apply(Image& image) const {
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            Color& color = image.At(x, y);
            color = Color(1.0 - color.GetRed(), 1.0 - color.GetGreen(), 1.0 - color.GetBlue());
        }
    }
}

MatrixFilter::MatrixFilter(std::vector<std::vector<double>> kernel) : kernel_(std::move(kernel)) {
    if (kernel_.empty()) {
        throw std::invalid_argument("Kernel cannot be empty");
    }
    const std::size_t width = kernel_.front().size();
    if (width == 0 || kernel_.size() % 2 == 0 || width % 2 == 0) {
        throw std::invalid_argument("Kernel must have odd non-zero dimensions");
    }
    for (const std::vector<double>& row : kernel_) {
        if (row.size() != width) {
            throw std::invalid_argument("Kernel rows must have equal width");
        }
    }
}

Image MatrixFilter::ApplyKernel(const Image& image) const {
    Image result(image.GetWidth(), image.GetHeight());
    const int kernel_height = static_cast<int>(kernel_.size());
    const int kernel_width = static_cast<int>(kernel_.front().size());
    const int center_y = kernel_height / 2;
    const int center_x = kernel_width / 2;

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            double red_sum = 0.0;
            double green_sum = 0.0;
            double blue_sum = 0.0;

            for (int kernel_y = 0; kernel_y < kernel_height; ++kernel_y) {
                for (int kernel_x = 0; kernel_x < kernel_width; ++kernel_x) {
                    const int source_x = x + kernel_x - center_x;
                    const int source_y = y + kernel_y - center_y;
                    const Color& source = image.GetClamped(source_x, source_y);
                    const double weight =
                        kernel_[static_cast<std::size_t>(kernel_y)][static_cast<std::size_t>(kernel_x)];
                    red_sum += source.GetRed() * weight;
                    green_sum += source.GetGreen() * weight;
                    blue_sum += source.GetBlue() * weight;
                }
            }

            result.At(x, y) = Color(red_sum, green_sum, blue_sum);
        }
    }

    return result;
}

const std::vector<std::vector<double>>& MatrixFilter::GetKernel() const {
    return kernel_;
}

SharpeningFilter::SharpeningFilter()
    : MatrixFilter({{0.0, -1.0, 0.0}, {-1.0, SharpenCenterWeight, -1.0}, {0.0, -1.0, 0.0}}) {
}

void SharpeningFilter::Apply(Image& image) const {
    image = ApplyKernel(image);
}

EdgeDetectionFilter::EdgeDetectionFilter(const double threshold)
    : MatrixFilter({{0.0, -1.0, 0.0}, {-1.0, EdgeCenterWeight, -1.0}, {0.0, -1.0, 0.0}}), threshold_(threshold) {
    if (threshold_ < 0.0 || threshold_ > 1.0) {
        throw std::invalid_argument("Edge threshold must be in range [0, 1]");
    }
}

void EdgeDetectionFilter::Apply(Image& image) const {
    GrayscaleFilter grayscale_filter;
    grayscale_filter.Apply(image);
    image = ApplyKernel(image);

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            const double value = image.At(x, y).GetRed();
            image.At(x, y) = value > threshold_ ? Color(1.0, 1.0, 1.0) : Color(0.0, 0.0, 0.0);
        }
    }
}

GaussianBlurFilter::GaussianBlurFilter(const double sigma) : sigma_(sigma) {
    if (sigma_ <= 0.0) {
        throw std::invalid_argument("Blur sigma must be positive");
    }
}

std::vector<double> GaussianBlurFilter::BuildKernel() const {
    const int radius = std::max(1, static_cast<int>(std::ceil(3.0 * sigma_)));
    std::vector<double> kernel(static_cast<std::size_t>(2 * radius + 1));
    double sum = 0.0;

    for (int offset = -radius; offset <= radius; ++offset) {
        const double exponent = -(static_cast<double>(offset * offset)) / (2.0 * sigma_ * sigma_);
        const double value = std::exp(exponent) / std::sqrt(2.0 * PI * sigma_ * sigma_);
        kernel[static_cast<std::size_t>(offset + radius)] = value;
        sum += value;
    }

    for (double& value : kernel) {
        value /= sum;
    }

    return kernel;
}

Image GaussianBlurFilter::ApplyHorizontalPass(const Image& image, const std::vector<double>& kernel) const {
    Image result(image.GetWidth(), image.GetHeight());
    const int radius = static_cast<int>(kernel.size() / 2);

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            double red_sum = 0.0;
            double green_sum = 0.0;
            double blue_sum = 0.0;

            for (int offset = -radius; offset <= radius; ++offset) {
                const Color& source = image.GetClamped(x + offset, y);
                const double weight = kernel[static_cast<std::size_t>(offset + radius)];
                red_sum += source.GetRed() * weight;
                green_sum += source.GetGreen() * weight;
                blue_sum += source.GetBlue() * weight;
            }

            result.At(x, y) = Color(red_sum, green_sum, blue_sum);
        }
    }

    return result;
}

Image GaussianBlurFilter::ApplyVerticalPass(const Image& image, const std::vector<double>& kernel) const {
    Image result(image.GetWidth(), image.GetHeight());
    const int radius = static_cast<int>(kernel.size() / 2);

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            double red_sum = 0.0;
            double green_sum = 0.0;
            double blue_sum = 0.0;

            for (int offset = -radius; offset <= radius; ++offset) {
                const Color& source = image.GetClamped(x, y + offset);
                const double weight = kernel[static_cast<std::size_t>(offset + radius)];
                red_sum += source.GetRed() * weight;
                green_sum += source.GetGreen() * weight;
                blue_sum += source.GetBlue() * weight;
            }

            result.At(x, y) = Color(red_sum, green_sum, blue_sum);
        }
    }

    return result;
}

void GaussianBlurFilter::Apply(Image& image) const {
    const std::vector<double> kernel = BuildKernel();
    image = ApplyVerticalPass(ApplyHorizontalPass(image, kernel), kernel);
}

GlowFilter::GlowFilter(const double threshold, const int radius, const double intensity)
    : threshold_(threshold), radius_(radius), intensity_(intensity) {
    if (threshold_ < 0.0 || threshold_ > 1.0) {
        throw std::invalid_argument("Glow threshold must be in range [0, 1]");
    }
    if (radius_ <= 0) {
        throw std::invalid_argument("Glow radius must be positive");
    }
    if (intensity_ < 0.0) {
        throw std::invalid_argument("Glow intensity must be non-negative");
    }
}

void GlowFilter::Apply(Image& image) const {
    Image result = image;

    for (int source_y = 0; source_y < image.GetHeight(); ++source_y) {
        for (int source_x = 0; source_x < image.GetWidth(); ++source_x) {
            const Color& source = image.At(source_x, source_y);
            const double luminance = GetLuminance(source);

            if (luminance < threshold_) {
                continue;
            }

            for (int dy = -radius_; dy <= radius_; ++dy) {
                for (int dx = -radius_; dx <= radius_; ++dx) {
                    const int target_x = source_x + dx;
                    const int target_y = source_y + dy;

                    if (!result.IsInside(target_x, target_y)) {
                        continue;
                    }

                    const double distance = std::sqrt(static_cast<double>(dx * dx + dy * dy));
                    if (distance > static_cast<double>(radius_)) {
                        continue;
                    }

                    const double factor = 1.0 - distance / static_cast<double>(radius_);
                    const double glow_strength = intensity_ * factor;

                    Color target = result.At(target_x, target_y);
                    target.SetRed(target.GetRed() + source.GetRed() * glow_strength);
                    target.SetGreen(target.GetGreen() + source.GetGreen() * glow_strength);
                    target.SetBlue(target.GetBlue() + source.GetBlue() * glow_strength);
                    result.At(target_x, target_y) = target;
                }
            }
        }
    }

    image = result;
}
ContrastFilter::ContrastFilter(const double coefficient) : coefficient_(coefficient) {
    if (coefficient_ < 0.0) {
        throw std::invalid_argument("Negative coefficient");
    }
}

void ContrastFilter::Apply(Image& image) const {
    for (int y = 0; y < image.GetHeight(); y++) {
        for (int x = 0; x < image.GetWidth(); x++) {
            constexpr double Mid = 0.5;
            Color& color = image.At(x, y);
            const double new_red = Mid + coefficient_ * (color.GetRed() - Mid);
            const double new_green = Mid + coefficient_ * (color.GetGreen() - Mid);
            const double new_blue = Mid + coefficient_ * (color.GetBlue() - Mid);

            color.SetRed(new_red);
            color.SetGreen(new_green);
            color.SetBlue(new_blue);
        }
    }
}