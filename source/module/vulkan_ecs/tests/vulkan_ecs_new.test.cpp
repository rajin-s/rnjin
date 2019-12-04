/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <rnjin.hpp>

#include rnjin_module( test )

#include rnjin_module( log )
#include rnjin_module( file )
#include rnjin_module( math )
#include rnjin_module( graphics )
#include rnjin_module( vulkan_ecs )
#include rnjin_module_subset( graphics, ecs )

using namespace rnjin;
using namespace rnjin::graphics;

test( vulkan_ecs )
{
    /* -------------------------------------------------------------------------- */
    /*                                Set Up Vulkan                               */
    /* -------------------------------------------------------------------------- */

    // Create main window
    window<GLFW> main_window( "rnjin: vulkan_ecs test", int2( 16, 9 ) * 30, true );

    // Create Vulkan API
    vulkan::api vk_api( "rnjin: vulkan_ecs test" );
    {
        vk_api.validation.enable( vulkan::api::messages::all );
        vk_api.initialize();
    }

    // Create Vulkan device and window surface
    vulkan::device vk_device( vk_api );
    vulkan::window_surface vk_surface( vk_device );
    {
        vk_surface.create_surface( main_window );
        vk_device.set_reference_surface( vk_surface );
        vk_device.initialize();
        vk_surface.initialize();
    }

    // Create Vulkan resource database and systems
    vulkan::resource_database vk_resources( vk_device );
    {
        vulkan::resource_database::initialization_info resource_db_info{
            4096 * sizeof( mesh::vertex ),                // vertex_buffer_space
            4096 * sizeof( mesh::index ),                 // index_buffer_space
            4096 * sizeof( mesh::vertex ),                // staging_buffer_space
            4096 * sizeof( ecs_material::instance_data ), // uniform_buffer_space
            512,                                          // max_descriptor_sets
        };
        vk_resources.initialize( resource_db_info );
    }

    /* -------------------------------------------------------------------------- */
    /*                               Set Up Systems                               */
    /* -------------------------------------------------------------------------- */

    // Create Vulkan renderer
    vulkan::renderer vk_renderer( vk_device, vk_surface );
    {
        vk_renderer.initialize();
    }

    vulkan::mesh_collector vk_mesh_collector( vk_resources );
    vulkan::material_collector vk_material_collector( vk_resources );
    vulkan::mesh_reference_collector vk_mesh_reference_collector;
    vulkan::material_reference_collector vk_material_reference_collector;
    vulkan::model_collector vk_model_collector;

    // Initialize Systems
    {
        vk_material_collector.temp_render_pass = vk_renderer.get_render_pass();

        vk_mesh_collector.initialize();
        vk_mesh_reference_collector.initialize();

        vk_material_collector.initialize();
        vk_material_reference_collector.initialize();

        vk_model_collector.initialize();
    }

    /* -------------------------------------------------------------------------- */
    /*                              Set Up Resources                              */
    /* -------------------------------------------------------------------------- */

    shader test_vsh( "test_vertex_shader", shader::type::vertex ), test_fsh( "test_fragment_shader", shader::type::fragment );
    {
        test_vsh.set_glsl( io::file::read_text_from( "shaders/test_vsh.glsl" ) );
        test_fsh.set_glsl( io::file::read_text_from( "shaders/test_fsh.glsl" ) );

        test_vsh.compile();
        test_fsh.compile();
    }
    mesh test_mesh = primitives::cube( 0.45 );

    /* -------------------------------------------------------------------------- */
    /*                               Set Up Entities                              */
    /* -------------------------------------------------------------------------- */

    entity ent1, ent2, ent3;

    // Add Components
    {
        ent1.add<ecs_mesh>( test_mesh );
        ent2.add<ecs_material>( &test_vsh, &test_fsh );

        ent3.add<ecs_model>();
        ent3.add<ecs_mesh::reference>( &ent1 );
        ent3.add<ecs_material::reference>( &ent2 );
    }

    /* -------------------------------------------------------------------------- */
    /*                             Simulate Main Loop                             */
    /* -------------------------------------------------------------------------- */

    let_mutable* material_pointer = ent2.get_mutable<ecs_material>();

    bool do_render = false;

    while ( true )
    {
        let window_closed = glfwWindowShouldClose( main_window.get_api_window() );
        if ( window_closed )
        {
            log::main.print( "Closing main window '\1'", main_window.get_title() );
            break;
        }

        glfwPollEvents();
        let advance = glfwGetKey( main_window.get_api_window(), GLFW_KEY_A );

        if ( advance )
        {
            if ( do_render )
            {
                do_render = false;

                // Update transformations
                {
                    let window_size = main_window.get_size();
                    let projection  = math::projection::orthographic_vertical( 2, float2( window_size.x, window_size.y ), -1, 1 );

                    material_pointer->get_mutable_instance_data().world_transform      = float4x4::identity();
                    material_pointer->get_mutable_instance_data().view_transform       = float4x4::identity();
                    material_pointer->get_mutable_instance_data().projection_transform = projection;

                    material_pointer->increment_instance_data_version();
                }

                vk_mesh_collector.update_all();
                vk_mesh_reference_collector.update_all();

                vk_material_collector.update_all();
                vk_material_reference_collector.update_all();

                vk_model_collector.update_all();

                vk_renderer.update_all();
            }
        }
        else
        {
            do_render = true;
        }
    }
}

/*

    TO DO

    - Clean up old vulkan ecs remnants
        - vulkan_resources (collector)
        - vulkan_resource_collector
        - vulkan_renderer.*.old

    - Clean up old resources
        - material
        - mesh
        - shader

*/