/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "material.hpp"

namespace rnjin::graphics
{
    material::material( const string& name ) : pass_member( name ), vertex_shader(), fragment_shader() {}
    material::material( const string&, const shader& vertex_shader, const shader& fragment_shader ) : pass_member( name ), pass_member( vertex_shader ), pass_member( fragment_shader )
    {
        material::events.material_loaded().send( *this );
    }

    material::~material()
    {
        material::events.material_destroyed().send( *this );
    }

    void material::write_data( io::file& file )
    {
        file.write_string( name );

        vertex_shader.save_to( file );
        fragment_shader.save_to( file );
    }

    void material::read_data( io::file& file )
    {
        name = file.read_string();

        vertex_shader.load_from( file );
        fragment_shader.load_from( file );
    }

    define_static_group( material::events );
} // namespace rnjin::graphics