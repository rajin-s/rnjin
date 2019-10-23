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

    // Create an empty mesh
    mesh::mesh() : vertices(), indices() {}

    // Create a non-empty mesh
    mesh::mesh( list<mesh::vertex> vertices, list<mesh::index> indices ) : vertices( vertices ), indices( indices ) {}

    // Destroy a mesh
    mesh::~mesh() {}

    // Save mesh data to a file
    void mesh::write_data( io::file& file )
    {
        file.write_buffer( vertices.data );
        file.write_buffer( indices.data );
    }

    // Read mesh data from a file
    void mesh::read_data( io::file& file )
    {
        // Read data from file
        vertices.data = file.read_buffer<vertex>();
        indices.data  = file.read_buffer<index>();
        
        vertices.version++;
        indices.version++;
    }
} // namespace rnjin::graphics