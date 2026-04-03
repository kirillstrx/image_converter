#pragma once

#include <vector>

#include "image.h"

class Filter {
public:
    virtual ~Filter() = default;
    virtual void Apply(Image& image) const = 0;
};

class CropFilter final : public Filter {
public:
    CropFilter(int width, int height);

    void Apply(Image& image) const override;

private:
    int width_;
    int height_;
};

class GrayscaleFilter final : public Filter {
public:
    void Apply(Image& image) const override;
};

class NegativeFilter final : public Filter {
public:
    void Apply(Image& image) const override;
};

class MatrixFilter : public Filter {
public:
    explicit MatrixFilter(std::vector<std::vector<double>> kernel);

protected:
    Image ApplyKernel(const Image& image) const;
    const std::vector<std::vector<double>>& GetKernel() const;

private:
    std::vector<std::vector<double>> kernel_;
};

class SharpeningFilter final : public MatrixFilter {
public:
    SharpeningFilter();

    void Apply(Image& image) const override;
};

class EdgeDetectionFilter final : public MatrixFilter {
public:
    explicit EdgeDetectionFilter(double threshold);

    void Apply(Image& image) const override;

private:
    double threshold_;
};

class GaussianBlurFilter final : public Filter {
public:
    explicit GaussianBlurFilter(double sigma);

    void Apply(Image& image) const override;

private:
    std::vector<double> BuildKernel() const;
    Image ApplyHorizontalPass(const Image& image, const std::vector<double>& kernel) const;
    Image ApplyVerticalPass(const Image& image, const std::vector<double>& kernel) const;

    double sigma_;
};

class GlowFilter : public Filter {
public:
    GlowFilter(double threshold, int radius, double intensity);

    void Apply(Image& image) const override;

private:
    double threshold_;
    int radius_;
    double intensity_;
};

class ContrastFilter : public Filter {
public:
    explicit ContrastFilter(double coefficient);

    void Apply(Image& image) const;

private:
    double coefficient_;
};
