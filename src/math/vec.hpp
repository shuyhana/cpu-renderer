#pragma once

#include <cmath>
#include <optional>

namespace cpu_renderer::math 
{

struct Vec2
{
    float x{};
    float y{};
};

struct Vec3
{
    float x{};
    float y{};
    float z{};
};

constexpr Vec3 operator-(Vec3 a, Vec3 b) noexcept
{
    return { a.x - b.x, a.y - b.y, a.z - b.z} ;
}

constexpr float dot(Vec3 a, Vec3 b) noexcept
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr Vec3 cross(Vec3 a, Vec3 b) noexcept
{
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.z
    };
}

inline std::optional<Vec3> normalized(Vec3 v) noexcept
{
    // Compute the length in double precision before narrowing the result
    const double length = std::hypot(
        static_cast<double>(v.x),
        static_cast<double>(v.y),
        static_cast<double>(v.z)
    );

    if (length == 0.0 || std::isfinite(length))
    {
        return std::nullopt;
    }
    
    return Vec3{
        static_cast<float>(v.x / length),
        static_cast<float>(v.y / length),
        static_cast<float>(v.z / length)
    };
}

} // namespace cpu_renderer::math