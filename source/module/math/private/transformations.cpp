/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "transformations.hpp"
#include "math_utilities.hpp"

namespace rnjin::core::math
{
    float4x4 axis_angle( float3 axis, float3 angle )
    {
        return float4x4();
    }

    namespace projection
    {
        float4x4 orthographic( float2 screen_size, float near_plane, float far_plane )
        {
            return float4x4(
                float4( 1.0 / screen_size.x, 0, 0, 0 ),                                                                     //
                float4( 0, -1.0 / screen_size.y, 0, 0 ),                                                                     //
                float4( 0, 0, 1.0 / ( far_plane - near_plane ), -( far_plane + near_plane ) / ( far_plane - near_plane ) ), //
                float4( 0, 0, 0, 1 )                                                                                        //
            );
        }

        float4x4 orthographic_vertical( float vertical_size, float2 screen_size, float near_plane, float far_plane )
        {
            screen_size.x = ( screen_size.x / screen_size.y ) * vertical_size;
            screen_size.y = vertical_size;
            return orthographic( screen_size, near_plane, far_plane );
        }
        float4x4 orthographic_horizontal( float horizontal_size, float2 screen_size, float near_plane, float far_plane )
        {
            screen_size.y = ( screen_size.y / screen_size.x ) * horizontal_size;
            screen_size.x = horizontal_size;
            return orthographic( screen_size, near_plane, far_plane );
        }
        float4x4 perspective( float field_of_view, float2 screen_size )
        {
            return float4x4();
        }
    } // namespace projection
} // namespace rnjin::core::math