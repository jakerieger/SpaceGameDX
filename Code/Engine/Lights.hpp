// Author: Jake Rieger
// Created: 1/20/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"

namespace x {
    static constexpr size_t kMaxPointLights = 16;
    static constexpr size_t kMaxSpotLights  = 16;
    static constexpr size_t kMaxAreaLights  = 16;

    struct alignas(16) DirectionalLight {
        Float4 direction = {0.0f, 0.0f, 0.0f, 1.0f};  // 16 bytes
        Float4 color     = {1.0f, 1.0f, 1.0f, 1.0f};  // 16 bytes
        f32 intensity    = 1.0f;                      // 4 bytes
        Float3 _pad1     = {0.0f, 0.0f, 0.0f};
        u32 castsShadow  = HLSL_TRUE;  // 4 bytes
        u32 enabled      = HLSL_TRUE;  // 4 bytes
        f32 _pad2[2]     = {0.0f, 0.0f};
    };

    struct alignas(16) PointLight {
        Float3 position = {0.0f, 0.0f, 0.0f};
        f32 _pad1       = 0.f;
        Float3 color    = {1.0f, 1.0f, 1.0f};
        f32 intensity   = 1.0f;
        f32 constant    = 1.0f;
        f32 linear      = 0.09f;
        f32 quadratic   = 0.032f;
        f32 radius      = 45.f;
        f32 _pad2       = 0.f;
        u32 castsShadow = HLSL_TRUE;
        u32 enabled     = HLSL_FALSE;
    };

    struct alignas(16) SpotLight {
        Float3 position  = {0.0f, 0.0f, 0.0f};  // Origin of the light
        f32 _pad1        = 0.f;
        Float3 direction = {0.0f, -1.0f, 0.0f};  // Direction the cone points
        f32 _pad2        = 0.f;
        Float3 color     = {1.0f, 1.0f, 1.0f};  // Light's color
        f32 intensity    = 1.0f;                // Overall brightness
        f32 innerAngle   = 0.8f;                // Inner cone angle (cosine)
        f32 outerAngle   = 0.6f;                // Outer cone angle (cosine)
        f32 range        = 50.0f;               // Maximum distance light travels
        u32 castsShadow  = HLSL_TRUE;
        u32 enabled      = HLSL_FALSE;
        f32 _pad3[3]     = {0.0f, 0.0f, 0.0f};
    };

    struct alignas(16) AreaLight {
        Float3 position   = {0.0f, 0.0f, 0.0f};   // Center position
        Float3 direction  = {0.0f, -1.0f, 0.0f};  // Normal direction
        Float3 color      = {1.0f, 1.0f, 1.0f};   // Light's color
        Float2 dimensions = {1.0f, 1.0f};         // Width and height of the area
        f32 intensity     = 1.0f;                 // Overall brightness
        u32 castsShadow   = HLSL_TRUE;
        u32 enabled       = HLSL_FALSE;
    };

    struct LightState {
        DirectionalLight Sun;
        PointLight PointLights[kMaxPointLights];
        SpotLight SpotLights[kMaxSpotLights];
        AreaLight AreaLights[kMaxAreaLights];
    };
}  // namespace x