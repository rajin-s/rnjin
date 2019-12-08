/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <rnjin.hpp>

#include "test/module.h"

#include "log/module.h"
#include "file/module.h"
#include "math/module.h"
#include "graphics/module.h"
#include "vulkan_ecs/module.h"
#include "graphics/ecs.h"

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

    entity ent1, ent2, ent3, ent4;

    // Add Components
    {
        ent1.add<ecs_mesh>( test_mesh );
        ent1.add<ecs_material>( &test_vsh, &test_fsh );
        ent2.add<ecs_material>( &test_vsh, &test_fsh );

        ent3.add<ecs_model>();
        ent3.add<ecs_mesh::reference>( &ent1 );
        ent3.add<ecs_material::reference>( &ent1 );
        
        ent4.add<ecs_model>();
        ent4.add<ecs_mesh::reference>( &ent1 );
        ent4.add<ecs_material::reference>( &ent2 );
    }

    /* -------------------------------------------------------------------------- */
    /*                             Simulate Main Loop                             */
    /* -------------------------------------------------------------------------- */

    let_mutable* material_pointer1 = ent1.get_mutable<ecs_material>();
    let_mutable* material_pointer2 = ent2.get_mutable<ecs_material>();

    bool do_render            = false;
    let_mutable projection    = math::perspective_projection( 90, 0.1, 5 );
    
    let_mutable transform1 = math::transform();
    transform1.set_position( float3( -1, 0, -1.5 ) );
    
    let_mutable transform2 = math::transform();
    transform2.set_position( float3( 0, 0, -2 ) );

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

        graphics::get_graphics_log().disable_flag( (uint) graphics::log_flag::verbose );

        if ( advance )
        {
            if ( do_render or true )
            {
                do_render = false;

                // Update transformations
                {
                    let window_size  = main_window.get_size();
                    let aspect_ratio = ( (float) window_size.x ) / ( (float) window_size.y );
                    projection.set_aspect_ratio( aspect_ratio );

                    transform1.rotate( float3( 0.07, 0.1, -0.03 ) );
                    transform2.rotate( float3( 0.03, -0.04, -0.07 ) );
                    
                    material_pointer1->get_mutable_instance_data().world_transform      = transform1.get_matrix();
                    material_pointer1->get_mutable_instance_data().view_transform       = float4x4::identity();
                    material_pointer1->get_mutable_instance_data().projection_transform = projection.get_matrix();
                    
                    material_pointer2->get_mutable_instance_data().world_transform      = transform2.get_matrix();
                    material_pointer2->get_mutable_instance_data().view_transform       = float4x4::identity();
                    material_pointer2->get_mutable_instance_data().projection_transform = projection.get_matrix();

                    material_pointer1->increment_instance_data_version();
                    material_pointer2->increment_instance_data_version();
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
    - Clean up old resources
        - material
        - mesh
        - shader

*/