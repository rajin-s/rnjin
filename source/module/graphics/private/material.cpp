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
        version(),           //
        uniforms_version(),  //
        uniforms()           //
    {}
    material::material( const string&, const shader& vertex_shader, const shader& fragment_shader )
      : pass_member( name ),            //
        pass_member( vertex_shader ),   //
        pass_member( fragment_shader ), //
        version(),                      //
        uniforms_version(),             //
        uniforms()                      //
    {}
    material::~material() {}

    void material::set_position( float3 position )
    {
        uniforms_version++;
        uniforms.world_matrix.rows[0].w += position.x;
        uniforms.world_matrix.rows[1].w += position.y;
        uniforms.world_matrix.rows[2].w += position.z;
    }
    void material::set_rotation_and_scale( float3 euler_angles, float3 scale )
    {
        uniforms_version++;
        uniforms.world_matrix.rows[0].x = scale.x;
        uniforms.world_matrix.rows[1].y = scale.y;
        uniforms.world_matrix.rows[2].z = scale.z;
    }
    void material::set_view( float3 position, float3 euler_angles )
    {
        uniforms_version++;
        uniforms.view_matrix.rows[0].w = position.x;
        uniforms.view_matrix.rows[1].w = position.y;
        uniforms.view_matrix.rows[2].w = position.z;
    }
    void material::set_projection( float4x4 matrix )
    {
        uniforms_version++;
        uniforms.projection_matrix = matrix;
    }

    void material::write_data( io::file& file ) const
    {
        file.write_string( name );

        vertex_shader.save_to( file );
        fragment_shader.save_to( file );

        file.write_var( uniforms );
    }

    void material::read_data( io::file& file )
    {
        name = file.read_string();

        vertex_shader.load_from( file );
        fragment_shader.load_from( file );

        uniforms = file.read_var<material_uniforms>();

        version++;
        uniforms_version++;
    }
    // define_static_group( material::events );
} // namespace rnjin::graphics