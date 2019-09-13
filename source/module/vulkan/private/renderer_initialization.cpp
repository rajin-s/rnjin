/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_window_surface.hpp"
#include "vulkan_renderer_internal.hpp"

namespace rnjin::graphics::vulkan
{
    renderer_internal::renderer_internal( api& api_instance ) : api_instance( api_instance )
    {
        vulkan_log_verbose.print( "Created Vulkan renderer internals" );
    }
    renderer_internal::~renderer_internal()
    {
        vulkan_log_verbose.print( "Destroyed Vulkan renderer internals" );
    }

    void renderer_internal::initialize()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan renderer initialization" );

        let has_window_targets = window_targets.size() > 0;
        if ( has_window_targets )
        {
            let& reference_surface = window_targets[0].get_vulkan_surface();
            create_device( &reference_surface );

            check_error_condition( return, vulkan_log_errors, device.initialized == false, "Vulkan device wasn't successfully initialized" );

            for ( auto& target : window_targets )
            {
                target.create_swapchain();
                target.create_render_pass();
                target.create_frame_buffers();
                target.create_command_buffers();
                target.initialize_synchronization();
            }
        }
        else
        {
            vulkan_log_verbose.print_warning( "Renderer initialized without window target(s), will not support presentation" );
            create_device( nullptr );
        }
    }

    void renderer_internal::clean_up()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan renderer cleanup" );

        // Wait for all current rendering operations to be finished before freeing resources
        device.vulkan_device.waitIdle();

        for ( auto& target : window_targets )
        {
            target.destroy_pipelines();
            target.destroy_synchronization();
            target.destroy_frame_buffers();
            target.destroy_render_pass();
            target.destroy_swapchain();
            target.destroy_surface();
        }
        destroy_device();
    }

    /***********************************
     * Vulkan structure initialization *
     ***********************************/

    void renderer_internal::create_device( const vk::SurfaceKHR* reference_surface )
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan device initialization" );

        device.initialized = false;

        let& instance          = get_vulkan_instance();
        let require_validation = api_instance.internal->validation.any_enabled();
        let require_present    = reference_surface != nullptr;

        tracked_subregion( vulkan_log_verbose, "Vulkan physical device selection" )
        {
            // Get all valid physical devices on the system
            list<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
            check_error_condition( return, vulkan_log_errors, devices.empty(), "Failed to find any Vulkan-compatible devices" );
            vulkan_log_verbose.print_additional( "Found \1 Vulkan-compatible device(s)", devices.size() );

            // Get the best physical device based on computed score
            int max_suitability = -1;
            vk::PhysicalDevice best_device;

            foreach ( device : devices )
            {
                let suitability = get_device_suitability( device, reference_surface, require_validation );
                if ( suitability > max_suitability )
                {
                    best_device     = device;
                    max_suitability = suitability;
                }
            }
            check_error_condition( return, vulkan_log_errors, not best_device, "Failed to find a suitable GPU" );

            device.physical_device = best_device;

            // Note: graphics queue is guarenteed to be present since it is checked during device selection
            device.find_queue_family_indices( reference_surface );

            check_error_condition( pass, vulkan_log_errors, device.queue.family_index.graphics < 0, "Failed to find graphics queue for device" );
            check_error_condition( pass, vulkan_log_errors, device.queue.family_index.compute < 0, "Failed to find compute queue for device" );
            check_error_condition( pass, vulkan_log_errors, require_present and device.queue.family_index.present < 0, "Failed to find present queue for device" );
        }

        // Create logical device
        tracked_subregion( vulkan_log_verbose, "Vulkan logical device creation" );
        {
            static const float queue_priority = 1.0; // default queue priority, doesn't change for now
            list<vk::DeviceQueueCreateInfo> queue_create_infos;

            // Get queue info for all unique, valid queues
            set<int> unique_queue_families = { device.queue.family_index.graphics, device.queue.family_index.present, device.queue.family_index.compute };
            foreach ( family : unique_queue_families )
            {
                if ( family >= 0 )
                {
                    queue_create_infos.push_back( vk::DeviceQueueCreateInfo(
                        {},             // flags
                        family,         // queueFamilyIndex
                        1,              // queueCount
                        &queue_priority // pQueuePriorities
                        )               //
                    );
                }
            }

            // Specify required device features (all default to false)
            vk::PhysicalDeviceFeatures features;

            // Get list of needed extensions
            list<const char*> enabled_extensions( required_device_extensions.begin(), required_device_extensions.end() );
            if ( require_present )
            {
                enabled_extensions.insert( enabled_extensions.end(), device_present_extensions.begin(), device_present_extensions.end() );
            }

            // Create logical device
            vk::DeviceCreateInfo create_info(
                {},                        // flags
                queue_create_infos.size(), // queueCreateInfoCount
                queue_create_infos.data(), // pQueueCreateInfos
                0,                         // enabledLayerCount   ==> api._internal->validation.get_active_layer_names()...
                nullptr,                   // ppEnabledLayerNames ==> api._internal->validation.get_active_layer_names()...
                enabled_extensions.size(), // enabledExtensionCount
                enabled_extensions.data(), // ppEnabledExtensionNames
                &features                  // pEnabledFeatures
            );
            device.vulkan_device = device.physical_device.createDevice( create_info );

            // Get queue handles
            const uint queue_index = 0;
            device.queue.graphics  = device.vulkan_device.getQueue( device.queue.family_index.graphics, queue_index );

            if ( device.queue.family_index.present >= 0 )
            {
                device.queue.present = device.vulkan_device.getQueue( device.queue.family_index.present, queue_index );
            }
        }

        // Create command pool
        tracked_subregion( vulkan_log_verbose, "Vulkan command pool initialization" );
        {
            vk::CommandPoolCreateInfo command_pool_info(
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer, // flags
                device.queue.family_index.graphics                  // queueFamilyIndex
            );
            device.command_pool = device.vulkan_device.createCommandPool( command_pool_info );
        }

        device.initialized = true;
    }

    void renderer_internal::destroy_device()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan device cleanup" );

        check_error_condition( return, vulkan_log_errors, device.initialized == false, "Destroying uninitialized Vulkan device" );

        device.vulkan_device.destroyCommandPool( device.command_pool );
        device.vulkan_device.destroy();
        device.initialized = false;
    }

    void renderer_internal::device_info::find_queue_family_indices( const vk::SurfaceKHR* surface )
    {
        queue.family_index.graphics = -1;
        queue.family_index.present  = -1;
        queue.family_index.compute  = -1;

        let queue_families = physical_device.getQueueFamilyProperties();
        for ( uint i : range( queue_families.size() ) )
        {
            let& family = queue_families[i];

            if ( family.queueCount > 0 )
            {
                let supports_graphics = family.queueFlags & vk::QueueFlagBits::eGraphics;
                let supports_compute  = family.queueFlags & vk::QueueFlagBits::eCompute;
                let supports_present  = surface != nullptr and physical_device.getSurfaceSupportKHR( i, *surface );

                if ( supports_graphics and queue.family_index.graphics < 0 )
                {
                    queue.family_index.graphics = i;
                }
                if ( supports_present and queue.family_index.present < 0 )
                {
                    queue.family_index.present = i;
                }
                if ( supports_compute and queue.family_index.compute < 0 )
                {
                    queue.family_index.compute = i;
                }
            }

            let found_graphics = queue.family_index.graphics >= 0;
            let found_present  = queue.family_index.present >= 0;
            let found_compute  = queue.family_index.compute >= 0;

            if ( found_graphics and found_compute and ( surface == nullptr or found_present ) )
            {
                break;
            }
        }
    }

    /* *** ** *** ** *** *** *** ** *** **
     * Physical Device Selection Helpers *
     * *** ** *** ** *** *** *** ** *** **/

    const list<const char*> required_device_extensions = {};
    const list<const char*> device_present_extensions  = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    struct device_queue_support_info
    {
        bool graphics = false;
        bool present  = false;
        bool compute  = false;
    };

    const device_queue_support_info get_device_queue_support( const vk::PhysicalDevice device, const vk::SurfaceKHR* surface )
    {
        device_queue_support_info support;

        let queue_families = device.getQueueFamilyProperties();

        for ( uint i : range( queue_families.size() ) )
        {
            let& family = queue_families[i];

            if ( family.queueCount > 0 )
            {
                let supports_graphics = family.queueFlags & vk::QueueFlagBits::eGraphics;
                let supports_compute  = family.queueFlags & vk::QueueFlagBits::eCompute;
                let supports_present  = surface != nullptr and device.getSurfaceSupportKHR( i, *surface );

                if ( supports_graphics )
                {
                    support.graphics = true;
                }
                if ( supports_compute )
                {
                    support.compute = true;
                }
                if ( supports_present )
                {
                    support.present = true;
                }
            }
        }

        return support;
    }

    const bool device_supports_extensions( const vk::PhysicalDevice device, const bool require_present, const bool require_validation )
    {
        const list<vk::ExtensionProperties> supported_extensions = device.enumerateDeviceExtensionProperties();
        set<string> unsupported_extensions;

        vulkan_log_verbose.print( "\1 required extension(s)", required_device_extensions.size() );
        for ( uint i : range( required_device_extensions.size() ) )
        {
            unsupported_extensions.insert( string( required_device_extensions[i] ) );
        }

        // Required presentation-specific extensions if needed
        if ( require_present )
        {
            foreach ( extension : device_present_extensions )
            {
                unsupported_extensions.insert( string( extension ) );
            }
        }

        // Remove supported extensions from unsupported set
        foreach ( extension : supported_extensions )
        {
            const string extension_name( extension.extensionName );
            unsupported_extensions.erase( extension_name );
        }

        // Report extension support information
        let unsupported_extension_count = unsupported_extensions.size();
        if ( unsupported_extension_count == 0 )
        {
            vulkan_log_verbose.print_additional( "All extensions supported" );
        }
        else
        {
            vulkan_log_errors.print_additional( "\1 unsupported extensions", unsupported_extension_count );
            foreach ( extension : unsupported_extensions )
            {
                vulkan_log_errors.print_additional( "  \1", extension );
            }
        }

        return unsupported_extensions.empty();
    }

    // Get a score representing how suitable a given physical device is (higher is better)
    // The most sutable device should be chosen to ensure best performance.
    // note: scoring is mostly arbitrary right now
    const int get_device_suitability( const vk::PhysicalDevice device, const vk::SurfaceKHR* surface, const bool require_validation )
    {
        static const int bad_device = -1;

        int score = 0;

        // Check device properties and features
        let properties = device.getProperties();
        let features   = device.getFeatures();

        if ( properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ) score += 1000;
        if ( features.geometryShader ) score += 200;
        if ( features.tessellationShader ) score += 200;

        let requires_present = surface != nullptr;
        let queue_support    = get_device_queue_support( device, surface );

        check_error_condition( return bad_device, vulkan_log_errors, queue_support.graphics == false, "Device (\1) doesn't support rendering", properties.deviceName );
        check_error_condition( return bad_device, vulkan_log_errors, requires_present and not queue_support.present, "Device (\1) doens't support surface display", properties.deviceName );

        const bool supports_extensions = device_supports_extensions( device, requires_present, require_validation );
        check_error_condition( return bad_device, vulkan_log_errors, supports_extensions == false, "Device (\1) doesn't support required extensions", properties.deviceName );

        if ( requires_present )
        {
            let supported_swapchain_formats       = device.getSurfaceFormatsKHR( *surface );
            let supported_swapchain_present_modes = device.getSurfacePresentModesKHR( *surface );

            check_error_condition( return bad_device, vulkan_log_errors, supported_swapchain_formats.empty(), "Device (\1) doesn't support any swapchain formats", properties.deviceName );
            check_error_condition( return bad_device, vulkan_log_errors, supported_swapchain_present_modes.empty(), "Device (\1) doesn't support any swapchain present modes", properties.deviceName );
        }

        vulkan_log_verbose.print_additional( "\1 (vendor: \2) score: \3", properties.deviceName, properties.vendorID, score );
        return score;
    }
} // namespace rnjin::graphics::vulkan