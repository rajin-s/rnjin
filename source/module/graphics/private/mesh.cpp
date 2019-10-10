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
    // note: don't fire the mesh loaded event
    mesh::mesh() : vertices(), indices() {}

    // Create a non-empty mesh
    mesh::mesh( list<mesh::vertex> vertices, list<mesh::index> indices ) : vertices( vertices ), indices( indices )
    {
        if ( has_data() )
        {
            mesh::events.mesh_loaded().send( *this );
        }
    }

    // Destroy a mesh
    // note: fire the mesh destroyed event if any data exists
    mesh::~mesh()
    {
        if ( has_data() )
        {
            mesh::events.mesh_destroyed().send( *this );
        }
    }

    // Save mesh data to a file
    void mesh::write_data( io::file& file )
    {
        file.write_buffer( vertices );
        file.write_buffer( indices );
    }

    // Read mesh data from a file
    // note: sends mesh destroyed and loaded events if data exists
    void mesh::read_data( io::file& file )
    {
        // Treat reading in data as effectively destroying the old mesh (if it exists)
        if ( has_data() )
        {
            mesh::events.mesh_destroyed().send( *this );
        }

        // Read data from file
        vertices = file.read_buffer<vertex>();
        indices  = file.read_buffer<index>();

        if ( has_data() )
        {
            mesh::events.mesh_loaded().send( *this );
        }
    }

    // Define static events structure
    define_static_group( mesh::events );

} // namespace rnjin::graphics