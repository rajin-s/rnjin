/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

namespace rnjin
{
    namespace core
    {
        // Vector Types
        struct float2
        {
            float2( float x, float y ) : x( x ), y( y ) {}
            float x, y;

            const float2 operator+(const float2 other)
            {
                return float2(x + other.x, y + other.y);
            }
            const float2 operator-(const float2 other)
            {
                return float2(x - other.x, y - other.y);
            }
        };

        struct float3
        {
            float3( float x, float y, float z ) : x( x ), y( y ), z( z ) {}
            float x, y, z;

            const float3 operator+(const float3 other)
            {
                return float3(x + other.x, y + other.y, z + other.z);
            }
            const float3 operator-(const float3 other)
            {
                return float3(x - other.x, y - other.y, z - other.z);
            }
        };
        
        struct float4
        {
            float4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
            float x, y, z, w;

            const float4 operator+(const float4 other)
            {
                return float4(x + other.x, y + other.y, z + other.z, w + other.w);
            }
            const float4 operator-(const float4 other)
            {
                return float4(x - other.x, y - other.y, z - other.z, w - other.w);
            }
        };
    } // namespace core
} // namespace rnjin