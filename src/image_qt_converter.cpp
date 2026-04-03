#include "image_qt_converter.h"

#include <QColor>

namespace {

    int ToByte(double value) {
        if (value < 0.0) {
            value = 0.0;
        }
        if (value > 1.0) {
            value = 1.0;
        }
        return static_cast<int>(value * 255.0 + 0.5);
    }

}  // namespace

QImage ToQImage(const Image& image) {
    QImage result(image.GetWidth(), image.GetHeight(), QImage::Format_RGB888);

    for (int y = 0; y < image.GetHeight(); ++y) {
        for (int x = 0; x < image.GetWidth(); ++x) {
            const Color& color = image.At(x, y);
            result.setPixelColor(
                x,
                y,
                QColor(
                    ToByte(color.GetRed()),
                    ToByte(color.GetGreen()),
                    ToByte(color.GetBlue())
                )
            );
        }
    }

    return result;
}