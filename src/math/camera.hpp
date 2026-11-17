#pragma once

#include <vec.hpp>

#include <cmath>
#include <optional>

namespace cpu_renderer::math 
{

struct CameraBasis
{
    Vec3 eye;
    Vec3 right;
    Vec3 up;
    Vec3 forward;
};

inline std::optional<CameraBasis> makeCameraBasis(Vec3 eye, Vec3 target, Vec3 upHint) noexcept
{
    const auto forward = normalized(target - eye);
    const auto up = normalized(upHint);
    if (!forward || !up)
    {
        return std::nullopt;
    }
    
    const Vec3 rightAxis = cross(*up, *forward);

    // Reject nearly parallel unit vectors before normalizing the cross product
    constexpr float minSinAngle = 1e-4f;
    if (dot(rightAxis, rightAxis) < minSinAngle * minSinAngle)
    {
        return std::nullopt;
    }
    
    const Vec3 right = *normalized(rightAxis);
    return CameraBasis{eye, right, cross(*forward, right), *forward};
}

inline Vec3 worldToView(const CameraBasis& camera, Vec3 point) noexcept
{
    const Vec3 delta = point - camera.eye;
    return {dot(delta, camera.right),
            dot(delta, camera.up),
            dot(delta, camera.forward)};
}

inline std::optional<Vec2> projectToScreen(Vec3 viewPoint, 
                                            float verticalFovRadians, 
                                            float nearZ, 
                                            int width, 
                                            int height) noexcept
{
    constexpr float pi = 3.14159265358979323846f;

    if (width <= 0 || height <= 0 || !(verticalFovRadians > 0.0f && verticalFovRadians < pi) ||
        !std::isfinite(nearZ) || nearZ <= 0.0f ||
        !std::isfinite(viewPoint.z) || viewPoint.z < nearZ)
    {
        return std::nullopt;
    }
    
    const float halfWidth = 0.5f * width;
    const float halfHeight = 0.5f * height;
    const float focal = halfHeight / std::tan(0.5f * verticalFovRadians);
    
    const Vec2 srceen{halfWidth + focal * (viewPoint.x / viewPoint.z),
                        halfHeight - focal * (viewPoint.y / viewPoint.z)};

    if (!std::isfinite(srceen.x) || !std::isfinite(srceen.y))
    {
        return std::nullopt;
    }
    
    return srceen;
}
    
}