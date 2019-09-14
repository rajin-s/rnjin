/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "resource.hpp"

namespace rnjin::graphics
{
    class mesh : public resource
    {
        public: // structures
        struct vertex
        {
            vertex();
            vertex(float3 position, float3 normal, float4 color, float2 uv);

            float3 position;
            float3 normal;
            float4 color;
            float2 uv;
        };

        struct vertex_info
        {
            static let constexpr position_size = sizeof( vertex::position );
            static let constexpr normal_size   = sizeof( vertex::normal );
            static let constexpr color_size    = sizeof( vertex::color );
            static let constexpr uv_size       = sizeof( vertex::uv );

            static let constexpr position_offset = offsetof( vertex, vertex::position );
            static let constexpr normal_offset   = offsetof( vertex, vertex::normal );
            static let constexpr color_offset    = offsetof( vertex, vertex::color );
            static let constexpr uv_offset       = offsetof( vertex, vertex::uv );
        };

        typedef uint index;

        public: // methods
        mesh( list<vertex> vertices, list<index> indices );
        ~mesh();

        public: // accessors
        let& get_vertices get_value( vertices );
        let& get_indices get_value( indices );

        protected: // inherited
        virtual void write_data( io::file& file );
        virtual void read_data( io::file& file );

        private: // members
        list<vertex> vertices;
        list<index> indices;
    };
} // namespace rnjin::graphics