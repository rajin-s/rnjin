/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "glfw.hpp"

#include "log.hpp"
#include "visual_ecs.hpp"
#include "primitives.hpp"

#include "vulkan_api.hpp"
#include "vulkan_device.hpp"
#include "vulkan_window_surface.hpp"
#include "vulkan_resource_collector.hpp"
#include "vulkan_resources.hpp"
#include "vulkan_renderer.hpp"

#include "test.hpp"

using namespace rnjin;
using namespace rnjin::graphics;

test( vulkan_ecs )
{
    // note: GLFW must be initialized before any Vulkan stuff can happen
    //       the whole windowing API structure should probably be redesigned

    window<GLFW> main_window( "rnjin: vulkan_resource_management test", int2( 128, 128 ), true );

    vulkan::api vk_api( "rnjin: vulkan_resource_management test" );
    {
        vk_api.validation.enable( vulkan::api::messages::all );
        // vk_api.extension.enable();

        vk_api.initialize();
    }

    vulkan::device vk_device( vk_api );
    vulkan::window_surface vk_surface( vk_device );
    {
        vk_surface.create_surface( main_window );
        vk_device.set_reference_surface( vk_surface );

        vk_device.initialize();
        vk_surface.initialize();
    }

    vulkan::renderer vk_renderer( vk_device, vk_surface );
    {
        vk_renderer.initialize();
    }
    debug_checkpoint( rnjin::log::main );

    // note: contains resource_database
    vulkan::resource_collector vk_resource_collector( vk_renderer );
    {
        const usize vertex_buffer_space  = sizeof( mesh::vertex ) * 1000;
        const usize index_buffer_space   = sizeof( mesh::index ) * 1000;
        const usize staging_buffer_space = sizeof( mesh::vertex ) * 500;

        vk_resource_collector.initialize( vertex_buffer_space, index_buffer_space, staging_buffer_space );
    }

    debug_checkpoint( rnjin::log::main );

    entity ent1, ent2;

    subregion
    {
        graphics::mesh new_cube = graphics::primitives::cube( 0.25 );
        new_cube.set_path( "test/cube.mesh" );
        new_cube.save();

        shader test_vertex   = shader( "Test Vertex Shader", shader::type::vertex );
        shader test_fragment = shader( "Test Fragment Shader", shader::type::fragment );
        test_vertex.set_glsl( io::file( "shaders/test_vsh.glsl", io::file::mode::read ).read_all_text() );
        test_vertex.compile();
        test_vertex.set_path( "test/vertex.shader" );
        test_vertex.save();

        test_fragment.set_glsl( io::file( "shaders/test_fsh.glsl", io::file::mode::read ).read_all_text() );
        test_fragment.compile();
        test_fragment.set_path( "test/fragment.shader" );
        test_fragment.save();

        material new_material = material( "Test Material", test_vertex, test_fragment );
        new_material.set_path( "test/new.material" );
        new_material.save();
    }

    ent1.add<model>( "test/cube.mesh", "test/new.material" );
    ent2.add<model>( "test/cube.mesh", "test/new.material" );

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
                vk_resource_collector.update_all();
                vk_renderer.update_all();

                do_render = false;
            }
        }
        else
        {
            do_render = true;
        }
    }

    debug_checkpoint( rnjin::log::main );

    // GLFW::clean_up();
}