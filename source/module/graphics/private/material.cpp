/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "material.hpp"

namespace rnjin::graphics
{
    material::material() {}
    material::material( const string& name )
      : pass_member( name ), //
        vertex_shader(),     //
        fragment_shader(),   //
        version()            //
    {}
    material::material( const string&, const shader& vertex_shader, const shader& fragment_shader )
      : pass_member( name ),            //
        pass_member( vertex_shader ),   //
        pass_member( fragment_shader ), //
        version()                       //
    {}
    material::~material() {}

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

        version++;
    }
    // define_static_group( material::events );
} // namespace rnjin::graphics