#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "filters.h"
#include "image.h"
#include "parser.h"

namespace {

constexpr double EPS = 1e-6;

constexpr double BLACK = 0.0;
constexpr double DARK = 0.1;
constexpr double LOW = 0.2;
constexpr double RedGrayscale = 0.299;
constexpr double MidLow = 0.25;
constexpr double MidDark = 0.3;
constexpr double MidLight = 0.4;
constexpr double MID = 0.5;
constexpr double MidHigh = 0.75;
constexpr double HIGH = 0.8;
constexpr double VeryHigh = 0.9;
constexpr double WHITE = 1.0;

constexpr int ImageSize3 = 3;
constexpr int ImageSize2 = 2;
constexpr int SinglePixelSize = 1;
constexpr int SmallCropSize = 2;
constexpr int LargeCropSize = 100;

constexpr double EdgeThreshold = DARK;
constexpr double BlurSigma = WHITE;
constexpr double GlowThreshold = VeryHigh;
constexpr double GlowIntensity = MID;

bool AreEqual(const double lhs, const double rhs) {
    return std::abs(lhs - rhs) < EPS;
}

void CheckColorClose(const Color& color, const double red, const double green, const double blue) {
    assert(AreEqual(color.GetRed(), red));
    assert(AreEqual(color.GetGreen(), green));
    assert(AreEqual(color.GetBlue(), blue));
}

Image MakeTestImage3x3() {
    Image image(ImageSize3, ImageSize3);

    image.At(0, 0) = Color(BLACK, BLACK, BLACK);
    image.At(1, 0) = Color(MID, MID, MID);
    image.At(2, 0) = Color(WHITE, WHITE, WHITE);

    image.At(0, 1) = Color(WHITE, BLACK, BLACK);
    image.At(1, 1) = Color(BLACK, WHITE, BLACK);
    image.At(2, 1) = Color(BLACK, BLACK, WHITE);

    image.At(0, 2) = Color(LOW, MidDark, MidLight);
    image.At(1, 2) = Color(DARK, DARK, DARK);
    image.At(2, 2) = Color(VeryHigh, VeryHigh, VeryHigh);

    return image;
}

void TestParserParsesPathsAndFilters() {
    const char* argv[] = {
        "./image_processor", "input.bmp", "output.bmp", "-crop", "10", "20", "-gs", "-glow", "0.8", "4", "0.5"};
    constexpr int Argc = static_cast<int>(std::size(argv));

    constexpr Parser Parser;
    const ParsedCommand command = Parser.Parse(Argc, argv);

    assert(command.input_path == "input.bmp");
    assert(command.output_path == "output.bmp");
    assert(command.filters.size() == ImageSize3);

    assert(command.filters[0].name == "crop");
    assert(command.filters[0].args.size() == SmallCropSize);
    assert(command.filters[0].args[0] == "10");
    assert(command.filters[0].args[1] == "20");

    assert(command.filters[1].name == "gs");
    assert(command.filters[1].args.empty());

    assert(command.filters[2].name == "glow");
    assert(command.filters[2].args.size() == ImageSize3);
    assert(command.filters[2].args[0] == "0.8");
    assert(command.filters[2].args[1] == "4");
    assert(command.filters[2].args[2] == "0.5");

    std::cout << "TestParserParsesPathsAndFilters: OK\n";
}

void TestParserRejectsTooFewArguments() {
    const char* argv[] = {"./image_processor", "input.bmp"};
    constexpr int Argc = static_cast<int>(std::size(argv));

    bool thrown = false;
    try {
        constexpr Parser Parser;
        static_cast<void>(Parser.Parse(Argc, argv));
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    assert(thrown);
    std::cout << "TestParserRejectsTooFewArguments: OK\n";
}

void TestGrayscaleFilter() {
    Image image(SinglePixelSize, SinglePixelSize);
    image.At(0, 0) = Color(WHITE, BLACK, BLACK);

    const GrayscaleFilter filter;
    filter.Apply(image);

    constexpr double Expected = RedGrayscale;
    CheckColorClose(image.At(0, 0), Expected, Expected, Expected);

    std::cout << "TestGrayscaleFilter: OK\n";
}

void TestNegativeFilter() {
    Image image(SinglePixelSize, SinglePixelSize);
    image.At(0, 0) = Color(LOW, MID, HIGH);

    const NegativeFilter filter;
    filter.Apply(image);

    CheckColorClose(image.At(0, 0), HIGH, MID, LOW);
    std::cout << "TestNegativeFilter: OK\n";
}

void TestCropFilterShrinksImage() {
    Image image = MakeTestImage3x3();

    const CropFilter filter(SmallCropSize, SmallCropSize);
    filter.Apply(image);

    assert(image.GetWidth() == SmallCropSize);
    assert(image.GetHeight() == SmallCropSize);

    CheckColorClose(image.At(0, 0), BLACK, BLACK, BLACK);
    CheckColorClose(image.At(1, 0), MID, MID, MID);
    CheckColorClose(image.At(0, 1), WHITE, BLACK, BLACK);
    CheckColorClose(image.At(1, 1), BLACK, WHITE, BLACK);

    std::cout << "TestCropFilterShrinksImage: OK\n";
}

void TestCropFilterLargeSizeKeepsImage() {
    Image image = MakeTestImage3x3();

    const CropFilter filter(LargeCropSize, LargeCropSize);
    filter.Apply(image);

    assert(image.GetWidth() == ImageSize3);
    assert(image.GetHeight() == ImageSize3);

    std::cout << "TestCropFilterLargeSizeKeepsImage: OK\n";
}

void TestSharpeningFilterKeepsFlatImageStable() {
    Image image(ImageSize2, ImageSize2);
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            image.At(x, y) = Color(MID, MID, MID);
        }
    }

    const SharpeningFilter filter;
    filter.Apply(image);

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            CheckColorClose(image.At(x, y), MID, MID, MID);
        }
    }

    std::cout << "TestSharpeningFilterKeepsFlatImageStable: OK\n";
}

void TestEdgeDetectionFilterFindsBrightPoint() {
    Image image(ImageSize3, ImageSize3);
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            image.At(x, y) = Color(BLACK, BLACK, BLACK);
        }
    }
    image.At(1, 1) = Color(WHITE, WHITE, WHITE);

    const EdgeDetectionFilter filter(EdgeThreshold);
    filter.Apply(image);

    bool has_white_pixel = false;
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            const Color& color = image.At(x, y);
            const bool is_white = AreEqual(color.GetRed(), WHITE) && AreEqual(color.GetGreen(), WHITE) &&
                                  AreEqual(color.GetBlue(), WHITE);
            if (is_white) {
                has_white_pixel = true;
            }
        }
    }

    assert(has_white_pixel);
    std::cout << "TestEdgeDetectionFilterFindsBrightPoint: OK\n";
}

void TestGaussianBlurFilterPreservesUniformImage() {
    Image image(ImageSize3, ImageSize3);
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            image.At(x, y) = Color(MidLow, MID, MidHigh);
        }
    }

    const GaussianBlurFilter filter(BlurSigma);
    filter.Apply(image);

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            CheckColorClose(image.At(x, y), MidLow, MID, MidHigh);
        }
    }

    std::cout << "TestGaussianBlurFilterPreservesUniformImage: OK\n";
}

void TestGlowFilterChangesPixelsNearBrightSource() {
    constexpr int GlowImageSize = 5;
    constexpr int GlowCenter = 2;
    constexpr int GlowRadius = 2;

    Image image(GlowImageSize, GlowImageSize);
    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            image.At(x, y) = Color(BLACK, BLACK, BLACK);
        }
    }
    image.At(GlowCenter, GlowCenter) = Color(WHITE, WHITE, WHITE);

    const GlowFilter filter(GlowThreshold, GlowRadius, GlowIntensity);
    filter.Apply(image);

    const Color& center = image.At(GlowCenter, GlowCenter);
    const Color& left = image.At(GlowCenter - 1, GlowCenter);
    const Color& top = image.At(GlowCenter, GlowCenter - 1);
    const Color& far_corner = image.At(0, 0);

    assert(center.GetRed() >= WHITE - EPS);
    assert(left.GetRed() > BLACK);
    assert(top.GetRed() > BLACK);
    assert(AreEqual(far_corner.GetRed(), BLACK));

    std::cout << "TestGlowFilterChangesPixelsNearBrightSource: OK\n";
}

void TestSinglePixelImageForFilters() {
    Image image(SinglePixelSize, SinglePixelSize);
    image.At(0, 0) = Color(MID, MID, MID);

    const SharpeningFilter sharp;
    sharp.Apply(image);

    const EdgeDetectionFilter edge(EdgeThreshold);
    edge.Apply(image);

    const GaussianBlurFilter blur(BlurSigma);
    blur.Apply(image);

    assert(image.GetWidth() == SinglePixelSize);
    assert(image.GetHeight() == SinglePixelSize);

    std::cout << "TestSinglePixelImageForFilters: OK\n";
}

}  // namespace

int main() {
    try {
        std::cout << "Running image_processor unit tests\n";

        TestParserParsesPathsAndFilters();
        TestParserRejectsTooFewArguments();
        TestGrayscaleFilter();
        TestNegativeFilter();
        TestCropFilterShrinksImage();
        TestCropFilterLargeSizeKeepsImage();
        TestSharpeningFilterKeepsFlatImageStable();
        TestEdgeDetectionFilterFindsBrightPoint();
        TestGaussianBlurFilterPreservesUniformImage();
        TestGlowFilterChangesPixelsNearBrightSource();
        TestSinglePixelImageForFilters();

        std::cout << "All tests passed\n";
    } catch (const std::exception& exception) {
        std::cerr << "Test failed with exception: " << exception.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }

    return 0;
}