/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <shaderc/shaderc.hpp>

#include "common.hpp"
#include "shader.hpp"

#include rnjin_module( console )

namespace rnjin
{
    namespace graphics
    {
        shader::shader( const string& name, const type shader_type ) : name( name ), shader_type( shader_type ) {}
        shader::shader() : resource() {}
        shader::~shader() {}

        text_resource& shader::get_glsl_resource()
        {
            return glsl;
        }

        void shader::set_glsl( const string& new_glsl )
        {
            glsl.content = new_glsl;
            spirv.clear();
        }

        void shader::compile()
        {
            spirv.clear();

            check_error_condition( return, graphics_log_errors, not has_glsl(), "Can't compile shader without GLSL ('\1')", get_name() );

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
                    check_error_condition( return, graphics_log_errors, invalid_shader_type == true, "Invalid shader type for compilation ('\1')", get_name() );
                    break;
                }
            }

            let result  = compiler.CompileGlslToSpv( glsl.content, shader_kind, get_name().c_str() );
            let success = result.GetCompilationStatus() == shaderc_compilation_status_success;

            if ( success )
            {
                graphics_log_verbose.print( "Compiled GLSL to SPIR-V ('\1': \2 warnings)", get_name(), result.GetNumWarnings() );
                for ( const spirv_char code : result )
                {
                    spirv.push_back( code );
                }
            }
            else
            {
                graphics_log_errors.print_error( "Failed to compile GLSL to SPIR-V ('\1': \2 errors)", get_name(), result.GetNumErrors() );
                graphics_log_errors << "\n" << result.GetErrorMessage();
            }
        }

        void shader::write_data( io::file& file ) const
        {
            file.write_var( shader_type );
            file.write_string( glsl.content );

            // glsl.save_to( file );

            const bool has_spirv = not spirv.empty();
            file.write_var( has_spirv );
            if ( has_spirv )
            {
                file.write_buffer( spirv );
            }
        }

        void shader::read_data( io::file& file )
        {
            shader_type  = file.read_var<type>();
            glsl.content = file.read_string();

            // glsl.load_from( file );

            const bool has_spirv = file.read_var<bool>();
            if ( has_spirv )
            {
                spirv = file.read_buffer<spirv_char>();
            }
        }

        /** *** ** *** ** ***
         * Console bindings *
         ** *** ** *** ** ***/

        void make_shader( const list<string>& params )
        {
            let& type        = params[0];
            let& name        = params[1];
            let& source_path = params[2];
            let& output_path = params[3];

            graphics_log_verbose.print( "Generating '\1' shader '\2' for source '\3' -> '\4'", type, name, source_path, output_path );
            shader::type shader_type = shader::type::vertex;

            if ( type == "fragment" )
            {
                shader_type = shader::type::fragment;
            }

            shader generated_shader( name, shader_type );
            generated_shader.set_path( output_path );

            text_resource& glsl = generated_shader.get_glsl_resource();
            glsl.set_path( source_path );

            generated_shader.save();
        }

        bind_console_parameters( "make-shader", "rnsh", "create a shader resource file with an external source file", make_shader, "vertex/fragment", "shader name", "source file path", "output file path" );
    } // namespace graphics
} // namespace rnjin