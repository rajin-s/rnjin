/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <rnjin.hpp>

#include rnjin_module( test )

#include rnjin_module( log )
#include rnjin_module( math )
#include rnjin_module( graphics )
#include rnjin_module( vulkan_ecs )
#include rnjin_module_subset( graphics, ecs )

using namespace rnjin;
using namespace rnjin::graphics;

/*

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
        // note: these are arbitrary numbers
        const usize vertex_buffer_space  = sizeof( mesh::vertex ) * 1000;
        const usize index_buffer_space   = sizeof( mesh::index ) * 1000;
        const usize staging_buffer_space = sizeof( mesh::vertex ) * 500;
        const usize uniform_buffer_space = sizeof( float4x4 ) * 400;
        const usize max_descriptor_sets  = 300;

        vulkan::resource_database::initialization_info resource_database_info{
            vertex_buffer_space,  // vertex_buffer_space;
            index_buffer_space,   // index_buffer_space;
            staging_buffer_space, // staging_buffer_space;
            uniform_buffer_space, // uniform_buffer_space;
            max_descriptor_sets   // max_descriptor_sets;
        };

        vk_resource_collector.initialize( resource_database_info );
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

    let_mutable& mat1 = ent1.get_mutable<model>()->get_material_mutable();

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
                {
                    mat1.set_position( float3( 0, 0.01, 0 ) );
                    mat1.set_rotation_and_scale( float3(), float3( 1, 1, 1 ) );
                    mat1.set_view( float3(), float3() );

                    let window_size = main_window.get_size();
                    mat1.set_projection( math::projection::orthographic_vertical( 2, float2( window_size.x, window_size.y ), -1, 1 ) );
                }

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

*/