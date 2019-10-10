/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "resource.hpp"
#include "event.hpp"

namespace rnjin::graphics
{
    class mesh : public resource
    {
        public: // structures
        struct vertex
        {
            vertex();
            vertex( float3 position, float3 normal, float4 color, float2 uv );

            float3 position;
            float3 normal;
            float4 color;
            float2 uv;
        };

        static group
        {
            static let constexpr vertex_size   = sizeof( vertex );
            static let constexpr position_size = sizeof( vertex::position );
            static let constexpr normal_size   = sizeof( vertex::normal );
            static let constexpr color_size    = sizeof( vertex::color );
            static let constexpr uv_size       = sizeof( vertex::uv );

            static let constexpr position_offset = offsetof( vertex, vertex::position );
            static let constexpr normal_offset   = offsetof( vertex, vertex::normal );
            static let constexpr color_offset    = offsetof( vertex, vertex::color );
            static let constexpr uv_offset       = offsetof( vertex, vertex::uv );
        }
        vertex_info;

        typedef uint16 index;

        public: // methods
        mesh();
        mesh( list<vertex> vertices, list<index> indices );

        ~mesh();

        public: // accessors
        let& get_vertices get_value( vertices );
        let& get_indices get_value( indices );
        let has_data get_value( not vertices.empty() );

        protected: // inherited
        virtual void write_data( io::file& file );
        virtual void read_data( io::file& file );

        private: // members
        list<vertex> vertices;
        list<index> indices;

        public: // static data
        static group
        {
            public: // accessors
            let_mutable& mesh_loaded get_mutable_value( mesh_loaded_event );
            let_mutable& mesh_destroyed get_mutable_value( mesh_destroyed_event );

            private: // members
            event<const mesh&> mesh_loaded_event{ "Mesh Loaded" };
            event<const mesh&> mesh_destroyed_event{ "Mesh Destroyed" };
        }
        events;
    };
} // namespace rnjin::graphics