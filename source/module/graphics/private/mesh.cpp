/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "mesh.hpp"

namespace rnjin::graphics
{
    mesh::vertex::vertex() {}
    mesh::vertex::vertex( float3 position, float3 normal, float4 color, float2 uv ) : position( position ), normal( normal ), color( color ), uv( uv ) {}

    mesh::mesh( list<mesh::vertex> vertices, list<mesh::index> indices ) : vertices( vertices ), indices( indices ) {}
    mesh::~mesh() {}

    void mesh::write_data( io::file& file )
    {
        file.write_buffer( vertices );
        file.write_buffer( indices );
    }

    void mesh::read_data( io::file& file )
    {
        vertices = file.read_buffer<vertex>();
        indices  = file.read_buffer<index>();
    }
} // namespace rnjin::graphics