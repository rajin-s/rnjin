/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include rnjin_module( resource )

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
        group mesh_vertices
        {
            public: // accessors
            let& get_data get_value( data );
            let get_version get_value( version );

            private: // members
            list<vertex> data;
            version_id version;

            friend class mesh;
            mesh_vertices() : data(), version() {}
            mesh_vertices( list<vertex> data ) : data( data ), version() {}
        }
        vertices;

        group mesh_indices
        {
            public: // accessors
            let& get_data get_value( data );
            let get_version get_value( version );

            private: // members
            list<index> data;
            version_id version;

            friend class mesh;
            mesh_indices() : data(), version() {}
            mesh_indices( list<index> data ) : data( data ), version() {}
        }
        indices;

        let has_data get_value( not vertices.data.empty() );

        protected: // inherited
        virtual void write_data( io::file& file ) const override;
        virtual void read_data( io::file& file ) override;
    };
} // namespace rnjin::graphics