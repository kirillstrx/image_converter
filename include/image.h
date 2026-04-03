#pragma once

#include <cstddef>
#include <vector>

class Color {
public:
    Color();
    Color(double red, double green, double blue);

    double GetRed() const;
    double GetGreen() const;
    double GetBlue() const;

    void SetRed(double red);
    void SetGreen(double green);
    void SetBlue(double blue);

    void Clamp();

private:
    double red_;
    double green_;
    double blue_;
};

class Image {
public:
    Image();
    Image(int width, int height);

    int GetWidth() const;
    int GetHeight() const;
    bool IsEmpty() const;
    bool IsInside(int x, int y) const;

    Color& At(int x, int y);
    const Color& At(int x, int y) const;
    const Color& GetClamped(int x, int y) const;

private:
    std::size_t GetIndex(int x, int y) const;

    int width_;
    int height_;
    std::vector<Color> pixels_;
};
