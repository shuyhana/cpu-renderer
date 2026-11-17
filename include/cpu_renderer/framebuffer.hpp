#pragma once

#include <cstdint>
#include <vector>

namespace cpu_renderer {

struct Rgb8 {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

class Framebuffer {
public:
    Framebuffer(int width, int height);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }

    // Coordinates range from zero to the corresponding extent minus one.
    Rgb8& pixel(int x, int y) noexcept;
    const Rgb8& pixel(int x, int y) const noexcept;

private:
    int width_;
    int height_;
    std::vector<Rgb8> pixels_;
};

void drawGradient(Framebuffer& framebuffer);

} // namespace cpu_renderer
