#include "cpu_renderer/framebuffer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace cpu_renderer {
namespace {

std::size_t pixelCount(int width, int height)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Framebuffer dimensions must be positive");
    }
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    if (w > std::numeric_limits<std::size_t>::max() / h) {
        throw std::length_error("Framebuffer dimensions are too large");
    }
    return w * h;
}

std::size_t pixelIndex(int x, int y, int width, [[maybe_unused]] int height) noexcept
{
    assert(x >= 0 && x < width && y >= 0 && y < height);
    return static_cast<std::size_t>(y) * width + x;
}

std::uint8_t encodeSrgb(float linear)
{
    const float c = std::clamp(linear, 0.0f, 1.0f);
    const float srgb = c <= 0.0031308f ? 12.92f * c
                                     : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
    return static_cast<std::uint8_t>(std::lround(srgb * 255.0f));
}

} // namespace

Framebuffer::Framebuffer(int width, int height)
    : width_(width), height_(height), pixels_(pixelCount(width, height))
{
}

Rgb8& Framebuffer::pixel(int x, int y) noexcept
{
    return pixels_[pixelIndex(x, y, width_, height_)];
}

const Rgb8& Framebuffer::pixel(int x, int y) const noexcept
{
    return pixels_[pixelIndex(x, y, width_, height_)];
}

void drawGradient(Framebuffer& framebuffer)
{
    // Use pixel centers in normalized screen coordinates.
    for (int y = 0; y < framebuffer.height(); ++y) {
        const float v = (y + 0.5f) / framebuffer.height();
        const auto green = encodeSrgb(0.65f * v);
        for (int x = 0; x < framebuffer.width(); ++x) {
            const float u = (x + 0.5f) / framebuffer.width();
            framebuffer.pixel(x, y) = {
                encodeSrgb(0.85f * u), green, encodeSrgb(0.75f - 0.55f * u)
            };
        }
    }
}

} // namespace cpu_renderer
