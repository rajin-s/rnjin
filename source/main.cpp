/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>

#include "core.hpp"
#include "glfw.hpp"
#include "graphics.hpp"
#include "log.hpp"
#include "script.hpp"
#include "vulkan_api.hpp"


// template <typename T>
// void print_bits( const void* value )
// {
//     size_t size = sizeof( T ) * 8;
//     T as_t      = *( (T*)value );
//     for ( int i = size - 1; i >= 0; i-- )
//     {
//         log::main << s( ( as_t >> i ) & 0x01 );
//     }
//     log::main << log::line();
// }

// void print_char( const char c )
// {
//     log::main.printf( "\1", { s( c ) } );
// }

// void main( int argc, char* argv[] )
// {
//     script::runtime rt = script::runtime();
//     script::compiled_script test_script( "test.script" );
//     script::execution_context exec( test_script, rt );

//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
//     exec.execute();
// }

using namespace rnjin;
using namespace rnjin::core;

void main( int argc, char* argv[] )
{
    // graphics::test();
    try
    {
        auto w = graphics::window<graphics::GLFW>( "My cool window", int2( 600, 600 ), true );
        graphics::vulkan::instance v( w, true );

        // test windowing
        while ( !glfwWindowShouldClose( w.get_api_window() ) )
        {
            glfwPollEvents();
        }

        log::main.print( "Exiting..." );
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    // try
    // {
    //     graphics::vulkan::initialize( &w );
    // }
    // catch ( std::exception e )
    // {
    //     log::main.print( e.what() );
    // }


    // graphics::vulkan::clean_up();
}