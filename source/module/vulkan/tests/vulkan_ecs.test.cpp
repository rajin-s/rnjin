/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "glfw.hpp"

#include "log.hpp"
#include "visual_ecs.hpp"

#include "vulkan_api.hpp"
#include "vulkan_device.hpp"
#include "vulkan_resource_collector.hpp"
#include "vulkan_resources.hpp"
#include "vulkan_renderer.hpp"

#include "test.hpp"

using namespace rnjin::graphics;

test( vulkan_resource_management )
{
    // note: GLFW must be initialized before any Vulkan stuff can happen
    //       the whole windowing API structure should probably be redesigned

    GLFW::initialize();

    vulkan::api vk_api( "rnjin: vulkan_resource_management test" );
    {
        vk_api.validation.enable( vulkan::api::messages::all );
        vk_api.extension.enable();

        vk_api.initialize();
    }


    vulkan::device vk_device( vk_api );
    {
        vk_device.initialize();
    }

    vulkan::resource_database vk_resources( vk_device );
    {
        vk_resources.initialize();
    }
    debug_checkpoint( rnjin::log::main );

    vulkan::resource_collector vk_resource_collector;
    {
        vk_resource_collector.initialize();
    }

    debug_checkpoint( rnjin::log::main );

    entity ent1;
    ent1.add<model>( "test/cube.mesh", "test/new.material" );

    vk_resource_collector.update_all();
    vk_resource_collector.update_all();

    debug_checkpoint( rnjin::log::main );

    GLFW::clean_up();
}