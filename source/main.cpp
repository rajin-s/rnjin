/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */
#include <rnjin.hpp>

#include <iostream>

#include rnjin_module( core )
#include rnjin_module( log )

#include "graphics/public/glfw.hpp"
// #include "render_view.hpp"
// #include "visual.hpp"
// #include "visual_ecs.hpp"

// #include "vulkan_api.hpp"
// #include "vulkan_renderer.hpp"
// #include "vulkan_ecs.hpp"

#include rnjin_module( console )

// #include "primitives.hpp"

#include rnjin_module( ecs )

using namespace rnjin;
using namespace rnjin::core;

static bool window_enabled = false;

void main( int argc, char* argv[] )
{
    subregion // parse command line arguments
    {
        list<string> args( argc - 1 );
        for ( uint i : range( 1, argc ) )
        {
            args[i - 1] = string( argv[i] );
        }
        console::parse_arguments( args );
    }

    if ( window_enabled )
    {
        try
        {
            using namespace graphics;

            window<GLFW> main_window( "RNJIN", int2( 128, 128 ), true );

            // vulkan::api vk_api( "rnjin.vulkan" );
            // {
            //     vk_api.validation.enable( vulkan::api::messages::all );
            //     vk_api.extension.enable();
            //     vk_api.initialize();
            // }

            // vulkan::renderer vk_renderer( vk_api );
            // {
            //     debug_checkpoint( log::main );
            //     vk_renderer.add_target( main_window );
            //     vk_renderer.initialize();
            //     debug_checkpoint( log::main );
            // }

            // tracked_subregion( log::main, "Resource creation" )
            // {
            //     graphics::mesh new_cube = graphics::primitives::cube( 0.25 );
            //     new_cube.set_path( "test/cube.mesh" );
            //     new_cube.save();

            //     graphics::shader test_vertex   = graphics::shader( "Test Vertex Shader", shader::type::vertex );
            //     graphics::shader test_fragment = graphics::shader( "Test Fragment Shader", shader::type::fragment );
            //     test_vertex.set_glsl( io::file( "shaders/test_vsh.glsl", io::file::mode::read ).read_all_text() );
            //     test_vertex.compile();
            //     test_vertex.set_path( "test/vertex.shader" );
            //     test_vertex.save();

            //     test_fragment.set_glsl( io::file( "shaders/test_fsh.glsl", io::file::mode::read ).read_all_text() );
            //     test_fragment.compile();
            //     test_fragment.set_path( "test/fragment.shader" );
            //     test_fragment.save();

            //     graphics::material new_material = graphics::material( "Test Material", test_vertex, test_fragment );
            //     new_material.set_path( "test/new.material" );
            //     new_material.save();
            // }
            // debug_checkpoint( log::main );

            // // ECS definitions
            // vulkan::resource_collector collect_vulkan_model_resources;
            // {
            //     collect_vulkan_model_resources.initialize();
            // }
            // entity test_entity;
            // {
            //     test_entity.add<model>( "test/cube.mesh", "test/new.material" );
            // }

            bool do_render = false;
            while ( not glfwWindowShouldClose( main_window.get_api_window() ) )
            {
                glfwPollEvents();
                let advance = glfwGetKey( main_window.get_api_window(), GLFW_KEY_A );
                let run     = glfwGetKey( main_window.get_api_window(), GLFW_KEY_S );
                if ( advance )
                {
                    if ( do_render or run )
                    {
                        // collect_vulkan_model_resources.update_all();
                        // vk_renderer.render( test_view );
                        do_render = false;
                    }
                }
                else
                {
                    do_render = true;
                }
            }

            debug_checkpoint( log::main );
        }
        catch ( const std::exception& e )
        {
            std::cerr << "\n[MAIN ERROR] " << e.what() << '\n';
        }
    }
}

void enable_window()
{
    window_enabled = true;
    log::main.print( "Enabling Vulkan window" );
}

bind_console_flag( "open_window", "w", "create a Vulkan renderer and window target", enable_window );