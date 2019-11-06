/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "vectors.hpp"

namespace rnjin::core::math
{
    float4x4 axis_angle( float3 axis, float3 angle );

    namespace projection
    {
        float4x4 orthographic_vertical( float vertical_size, float2 screen_size, float near_plane, float far_plane );
        float4x4 orthographic_horizontal( float horizontal_size, float2 screen_size, float near_plane, float far_plane );
        float4x4 perspective( float field_of_view, float2 screen_size );
    } // namespace projection
} // namespace rnjin::core::math