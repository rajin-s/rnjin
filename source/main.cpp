/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>

#include "core.hpp"
#include "log.hpp"

#include "glfw.hpp"
#include "render_view.hpp"
#include "visual.hpp"

#include "vulkan_api.hpp"
#include "vulkan_renderer.hpp"

#include "console.hpp"

using namespace rnjin;
using namespace rnjin::core;

static bool window_enabled = false;
void main( int argc, char* argv[] )
{
    list<string> args( argc - 1 );
    for ( uint i : range( 1, argc ) )
    {
        args[i - 1] = string( argv[i] );
    }
    console::parse_arguments( args );

    if ( window_enabled )
    {
        try
        {
            using namespace graphics;

            window<GLFW> main_window( "RNJIN", int2( 128, 128 ), true );

            vulkan vk_api( "rnjin.vulkan" );
            {
                vk_api.validation.enable( vulkan::messages::all );
                vk_api.extension.enable();
                vk_api.initialize();
            }

            debug_checkpoint( log::main );
            renderer<vulkan> vk_renderer( vk_api );
            {
                vk_renderer.add_target( main_window );
                vk_renderer.initialize();
            }
            debug_checkpoint( log::main );

            render_view test_view;
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
                        vk_renderer.render( test_view );
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