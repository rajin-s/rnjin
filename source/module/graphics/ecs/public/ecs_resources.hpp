/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

/* -------------------------------------------------------------------------- */
/*                                    TEST                                    */
/* -------------------------------------------------------------------------- */

#pragma once
#include <rnjin.hpp>

#include third_party_library( "shaderc/shaderc.hpp" )

#include rnjin_module( core )
#include rnjin_module( ecs )
#include rnjin_module_public( graphics, common )

// Temporarily pull in some stuff from old resources
#include rnjin_module_public( graphics, mesh )
#include rnjin_module_public( graphics, shader )
#include rnjin_module_public( graphics, material )

namespace rnjin::graphics
{
    component_class( ecs_shader )
    {
        public: // types
        typedef uint32_t spirv_char;
        enum type
        {
            vertex,
            fragment
        };

        public: // methods
        ecs_shader( type shader_type, const string& glsl_text )
          : pass_member( glsl_text ),  //
            pass_member( shader_type ) //
        {}
        ~ecs_shader() {}

        let has_spirv get_value( not spirv_text.empty() );

        void compile()
        {
            spirv_text.clear();
            check_error_condition( return, graphics_log_errors, glsl_text.empty(), "Can't compile shader without GLSL" );

            {
                shaderc::Compiler compiler;
                shaderc_shader_kind shader_kind;

                switch ( shader_type )
                {
                    case type::vertex:
                    {
                        shader_kind = shaderc_vertex_shader;
                        break;
                    }
                    case type::fragment:
                    {
                        shader_kind = shaderc_fragment_shader;
                        break;
                    }
                    default:
                    {
                        let invalid_shader_type = true;
                        check_error_condition( return, graphics_log_errors, invalid_shader_type == true, "Invalid shader type for compilation" );
                        break;
                    }
                }

                let result  = compiler.CompileGlslToSpv( glsl_text, shader_kind, "ecs_shader" );
                let success = result.GetCompilationStatus() == shaderc_compilation_status_success;

                if ( success )
                {
                    graphics_log_verbose.print( "Compiled GLSL to SPIR-V (\1 warnings)", result.GetNumWarnings() );
                    for ( const spirv_char code : result )
                    {
                        spirv_text.push_back( code );
                    }
                }
                else
                {
                    graphics_log_errors.print_error( "Failed to compile GLSL to SPIR-V (\1 errors)", result.GetNumErrors() );
                    graphics_log_errors << "\n" << result.GetErrorMessage();
                }
            }
        }

        private: // members
        type shader_type;
        string glsl_text;
        list<spirv_char> spirv_text;
    };

    component_class( ecs_material )
    {
        public: // methods
        ecs_material( const shader* vertex_shader, const shader* fragment_shader )
          : pass_member( vertex_shader ),  //
            pass_member( fragment_shader ) //
        {}
        ~ecs_material() {}

        void increment_version()
        {
            version++;
        }
        void increment_instance_data_version()
        {
            instance_data_version++;
        }

        public: // types
        struct instance_data
        {
            float4x4 world_transform;
            float4x4 view_transform;
            float4x4 projection_transform;
        };

        public: // accessors
        let get_version get_value( version );
        let get_instance_data_version get_value( instance_data_version );
        let& get_vertex_shader get_value( vertex_shader );
        let& get_fragment_shader get_value( fragment_shader );

        let& get_instance_data get_value( uniforms );
        let_mutable& get_mutable_instance_data get_mutable_value( uniforms );

        private: // members
        version_id version;
        version_id instance_data_version;

        const shader* vertex_shader;
        const shader* fragment_shader;

        instance_data uniforms;

        public: // static methods
        static constexpr usize get_instance_data_size()
        {
            return sizeof( instance_data );
        }
    };

    component_class( ecs_mesh )
    {
        public: // methods
        ecs_mesh( const mesh& src )
        {
            vertices.data = src.vertices.get_data();
            indices.data  = src.indices.get_data();
        }

        public: // accessors
        group
        {
            public: // accessors
            let get_version get_value( version );
            let& get_data get_value( data );

            private: // members
            version_id version;
            list<mesh::vertex> data;

            friend class ecs_mesh;
        }
        vertices;

        group
        {
            public: // accessors
            let get_version get_value( version );
            let& get_data get_value( data );

            private: // members
            version_id version;
            list<mesh::index> data;

            friend class ecs_mesh;
        }
        indices;

        private: // members
    };

    component_class( ecs_model ){
        // public: // methods
        // ecs_model( ecs_mesh::reference mesh_reference, ecs_material::reference material_reference )
        //   : pass_member( mesh_reference ),    //
        //     pass_member( material_reference ) //
        // {}

        // public: // accessors
        // let& get_mesh get_value( mesh_reference );
        // let& get_material get_value( material_reference );

        // private: // members
        // ecs_mesh::reference mesh_reference;
        // ecs_material::reference material_reference;
    };

    /* -------------------------------------------------------------------------- */
    /*                                   Systems                                  */
    /* -------------------------------------------------------------------------- */

    class shader_compiler : ecs::system<write_to<ecs_shader>>
    {
        protected:
        void update( entity_components& components ) override
        {
            let_mutable& sh = components.writable<ecs_shader>();
            if ( not sh.has_spirv() )
            {
                sh.compile();
            }
        }
    };
} // namespace rnjin::graphics

namespace reflection
{
    auto_reflect_component( rnjin::graphics, ecs_mesh );
    auto_reflect_component( rnjin::graphics, ecs_material );
    auto_reflect_component( rnjin::graphics, ecs_model );
} // namespace reflection