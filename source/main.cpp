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

#include "primitives.hpp"

#include "ecs.hpp"

using namespace rnjin;
using namespace rnjin::core;
using namespace rnjin::ecs;

static bool window_enabled = false;

component_class( component_a )
{
    public:
    component_a( int bar ) : pass_member( bar ) {}
    ~component_a() {}

    int bar;
};

component_class( component_b )
{
    public:
    component_b( float foo ) : pass_member( foo ) {}
    ~component_b() {}

    float foo;
};

class test_system : public ecs::system<read_from<component_a>, write_to<component_b>>
{
    void define() {}
    void initialize() {}

    void update( entity_components& components )
    {
        let bar = components.readable<component_a>().bar;
        let foo = components.writable<component_b>().foo;

        // Invalid, since the system is only defined on read_from<component_a>
        // this will generate a type error at compile-time
        // let& write_a = components.writable<component_a>();

        log::main.print( "test_system: (\1, \2)", bar, foo );
    }
};

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

    subregion
    {
        ecs::entity ent1, ent2, ent3, ent4;

        ent3.add<component_a>( 6 );
        ent2.add<component_a>( 4 );
        ent4.add<component_a>( 8 );

        ent1.add<component_b>( 7.5 );
        ent2.add<component_b>( 5.5 );
        ent4.add<component_b>( 9.5 );

        test_system sys;
        sys.update_all();

        ent2.remove<component_b>();
        sys.update_all();

        ent4.remove<component_a>();
        sys.update_all();

        // test_system is defined on { component_a, component_b }
        // so it will only iterate over entities with both component types (in order of ID)
        // expected output:
        //      test_system: (4, 5.5) <- ent2
        //      test_system: (8, 9.5) <- ent4
        //      test_system: (8, 9.5) <- ent4 (after removing component_b from ent2)
        //      end                   <- no valid entities left after removing component_a from ent4
    }

    if ( window_enabled )
    {
        try
        {
            using namespace graphics;

            window<GLFW> main_window( "RNJIN", int2( 128, 128 ), true );

            vulkan::api vk_api( "rnjin.vulkan" );
            {
                vk_api.validation.enable( vulkan::api::messages::all );
                vk_api.extension.enable();
                vk_api.initialize();
            }

            debug_checkpoint( log::main );
            vulkan::renderer vk_renderer( vk_api );
            {
                vk_renderer.add_target( main_window );
                vk_renderer.initialize();
            }

            debug_checkpoint( log::main );
            graphics::mesh new_cube = graphics::primitives::cube( 0.25 );
            new_cube.set_path( "test/cube.mesh" );
            new_cube.save();

            graphics::shader test_vertex   = graphics::shader( "Test Vertex Shader", shader::type::vertex );
            graphics::shader test_fragment = graphics::shader( "Test Fragment Shader", shader::type::fragment );
            test_vertex.set_glsl( io::file( "shaders/test_vsh.glsl", io::file::mode::read ).read_all_text() );
            test_fragment.set_glsl( io::file( "shaders/test_fsh.glsl", io::file::mode::read ).read_all_text() );
            test_vertex.compile();
            test_fragment.compile();

            graphics::material new_material = graphics::material( "Test Material", test_vertex, test_fragment );
            new_material.set_path( "test/new.material" );
            new_material.save();

            debug_checkpoint( log::main );

            render_view test_view;
            test_view.add_item( new_cube, new_material );

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