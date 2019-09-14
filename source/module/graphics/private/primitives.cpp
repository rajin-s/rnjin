/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "primitives.hpp"

namespace rnjin::graphics::primitives
{
    mesh cube( const float radius )
    {
        using vertex = mesh::vertex;

        let points = list<float3>{
            float3( -radius, -radius, -radius ), // [0] bottom left back
            float3( -radius, -radius, +radius ), // [1] bottom left front
            float3( +radius, -radius, +radius ), // [2] bottom right front
            float3( +radius, -radius, -radius ), // [3] bottom right back
            float3( -radius, +radius, -radius ), // [4] top left back
            float3( -radius, +radius, +radius ), // [5] top left front
            float3( +radius, +radius, +radius ), // [6] top right front
            float3( +radius, +radius, -radius ), // [7] top right back
        };

        static let directions = list<float3>{
            float3( +1, 0, 0 ), // [0] right
            float3( -1, 0, 0 ), // [1] left
            float3( 0, +1, 0 ), // [2] up
            float3( 0, -1, 0 ), // [3] down
            float3( 0, 0, +1 ), // [4] forward
            float3( 0, 0, -1 ), // [5] back
        };

        static let white = float4( 1, 1, 1, 1 );

        static let uvs = list<float2>{
            float2( 0, 0 ), // [0] bottom left
            float2( 0, 1 ), // [1] bottom right
            float2( 1, 0 ), // [2] top left
            float2( 1, 1 ), // [3] top right
        };

        let vertices = list<mesh::vertex>{
            // Left face
            vertex( points[0], directions[1], white, uvs[1] ),
            vertex( points[1], directions[1], white, uvs[0] ),
            vertex( points[4], directions[1], white, uvs[3] ),
            vertex( points[5], directions[1], white, uvs[2] ),

            // Back face
            vertex( points[0], directions[5], white, uvs[1] ),
            vertex( points[3], directions[5], white, uvs[0] ),
            vertex( points[4], directions[5], white, uvs[3] ),
            vertex( points[7], directions[5], white, uvs[2] ),

            // Top face
            vertex( points[4], directions[2], white, uvs[1] ),
            vertex( points[7], directions[2], white, uvs[0] ),
            vertex( points[5], directions[2], white, uvs[3] ),
            vertex( points[6], directions[2], white, uvs[2] ),

            // Front face
            vertex( points[1], directions[4], white, uvs[1] ),
            vertex( points[2], directions[4], white, uvs[0] ),
            vertex( points[5], directions[4], white, uvs[3] ),
            vertex( points[6], directions[4], white, uvs[2] ),

            // Right face
            vertex( points[3], directions[0], white, uvs[1] ),
            vertex( points[2], directions[0], white, uvs[0] ),
            vertex( points[7], directions[0], white, uvs[3] ),
            vertex( points[6], directions[0], white, uvs[2] ),

            // Bottom face
            vertex( points[3], directions[3], white, uvs[1] ),
            vertex( points[1], directions[3], white, uvs[0] ),
            vertex( points[2], directions[3], white, uvs[3] ),
            vertex( points[0], directions[3], white, uvs[2] ),
        };

        // clang-format off
        static let indices = list<mesh::index>{
            // Left face
            0, 1, 2,
            1, 3, 2,

            // Back face
            4, 5, 6,
            5, 7, 6,
            
            // Top face
            8, 9, 10,
            9, 11, 10,
            
            // Front face
            12, 13, 14,
            13, 15, 14,
            
            // Right face
            16, 17, 18,
            17, 19, 18,
            
            // Bottom face
            20, 21, 22,
            21, 23, 22,
        };
        // clang-format on

        return mesh( vertices, indices );
    }
} // namespace rnjin::graphics::primitives