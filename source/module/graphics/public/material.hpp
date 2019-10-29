/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "resource.hpp"
#include "shader.hpp"
#include "event.hpp"

namespace rnjin::graphics
{
    class material : public resource
    {
        public: // methods
        material();
        material( const string& name );
        material( const string& name, const shader& vertex_shader, const shader& fragment_shader );
        ~material();

        // test methods
        void set_position( float3 position );
        void set_rotation_and_scale( float3 euler_angles, float3 scale );
        void set_view( float3 position, float3 euler_angles );
        void set_projection( float4x4 matrix );

        public: // accessors
        let& get_name get_value( name );
        let& get_vertex_shader get_value( vertex_shader );
        let& get_fragment_shader get_value( fragment_shader );

        let inline get_version get_value( version );
        let inline get_uniforms_version get_value( uniforms_version );

        let& get_uniforms get_value( uniforms );
        static constexpr usize get_uniforms_size()
        {
            return sizeof( material_uniforms );
        }

        protected: // inherited
        virtual void write_data( io::file& file );
        virtual void read_data( io::file& file );

        protected: // members
        string name;

        // TODO: separate shaders into references
        //       handle uniforms properly
        shader vertex_shader;
        shader fragment_shader;

        struct material_uniforms
        {
            material_uniforms() : world_matrix(), view_matrix(), projection_matrix() {}
            float4x4 world_matrix, view_matrix, projection_matrix;
        } uniforms;

        version_id version;
        version_id uniforms_version;
    };
} // namespace rnjin::graphics