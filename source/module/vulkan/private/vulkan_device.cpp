/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_api_internal.hpp"
#include "vulkan_window_surface.hpp"
#include "vulkan_device.hpp"

namespace rnjin::graphics::vulkan
{
    // Helper functions and constant data
    extern const list<const char*> required_device_extensions;
    extern const list<const char*> device_present_extensions;

    const vk::SurfaceFormatKHR get_best_surface_format( const list<vk::SurfaceFormatKHR>& available_formats );
    const vk::PresentModeKHR get_best_present_mode( const list<vk::PresentModeKHR>& available_present_modes );
    const uint2 get_best_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities, const uint2 window_size );
    const int get_device_suitability( const vk::PhysicalDevice device, const vk::SurfaceKHR* reference_surface, const bool require_validation );

    // Vulkan device wrapper
    device::device( api& api_instance ) : pass_member( api_instance ), is_initialized( false ), reference_surface( nullptr )
    {
        vulkan_log_verbose.print( "Created Vulkan device wrapper" );
    }
    device::~device()
    {
        clean_up();
        vulkan_log_verbose.print( "Destroying Vulkan device wrapper" );
        check_error_condition( pass, vulkan_log_errors, is_initialized == true, "Destroying Vulkan device wrapper without cleaning up resources" );
    }

    // Fetch the Vulkan instance from the API instance
    const vk::Instance& device::get_vulkan_instance() const
    {
        return api_instance.internal->instance.vulkan_instance;
    }

    void device::set_reference_surface( const window_surface& surface )
    {
        reference_surface = &surface.get_vulkan_surface();
    }

    void device::initialize()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan device wrapper initialization" );

        let& instance          = get_vulkan_instance();
        let require_validation = api_instance.internal->validation.any_enabled();
        let require_present    = reference_surface != nullptr && reference_surface;

        // Select the best physical device, if multiple are present
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

            physical_device = best_device;

            // Note: graphics queue is guaranteed to be present since it is checked during device selection
            find_queue_family_indices();

            check_error_condition( pass, vulkan_log_errors, queue.family_index.graphics() < 0, "Failed to find graphics queue for device" );
            check_error_condition( pass, vulkan_log_errors, queue.family_index.compute() < 0, "Failed to find compute queue for device" );
            check_error_condition( pass, vulkan_log_errors, require_present and queue.family_index.present() < 0, "Failed to find present queue for device" );
        }

        // Create logical device
        tracked_subregion( vulkan_log_verbose, "Vulkan logical device creation" );
        {
            static const float queue_priority = 1.0; // default queue priority, doesn't change for now
            list<vk::DeviceQueueCreateInfo> queue_create_infos;

            // Get queue info for all unique, valid queues
            set<int> unique_queue_families = { queue.family_index.graphics(), queue.family_index.present(), queue.family_index.compute() };
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
            vulkan_device = physical_device.createDevice( create_info );
            check_error_condition( return, vulkan_log_errors, not vulkan_device, "Failed to create Vulkan logical device" );

            // Get queue handles
            const uint queue_index = 0;

            queue.graphics_queue = vulkan_device.getQueue( queue.family_index.graphics(), queue_index );
            if ( queue.family_index.present() >= 0 )
            {
                queue.present_queue = vulkan_device.getQueue( queue.family_index.present(), queue_index );
            }
        }

        // Create command pools
        tracked_subregion( vulkan_log_verbose, "Vulkan command pool initialization" );
        {
            vk::CommandPoolCreateInfo command_pool_info(
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer, // flags
                queue.family_index.graphics()                       // queueFamilyIndex
            );
            command_pool.main_pool = vulkan_device.createCommandPool( command_pool_info );

            vk::CommandPoolCreateInfo transfer_command_pool_info(
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, // flags
                queue.family_index.graphics()                                                                   // queueFamilyIndex
            );
            command_pool.transfer_pool = vulkan_device.createCommandPool( transfer_command_pool_info );
        }

        is_initialized = true;
    }

    void device::find_queue_family_indices()
    {
        queue.family_index.graphics_index = -1;
        queue.family_index.present_index  = -1;
        queue.family_index.compute_index  = -1;

        let queue_families = physical_device.getQueueFamilyProperties();
        for ( uint i : range( queue_families.size() ) )
        {
            let& family = queue_families[i];

            if ( family.queueCount > 0 )
            {
                let supports_graphics = family.queueFlags & vk::QueueFlagBits::eGraphics;
                let supports_compute  = family.queueFlags & vk::QueueFlagBits::eCompute;
                let supports_present  = reference_surface != nullptr and physical_device.getSurfaceSupportKHR( i, *reference_surface );

                if ( supports_graphics and queue.family_index.graphics_index < 0 )
                {
                    queue.family_index.graphics_index = i;
                }
                if ( supports_present and queue.family_index.present_index < 0 )
                {
                    queue.family_index.present_index = i;
                }
                if ( supports_compute and queue.family_index.compute_index < 0 )
                {
                    queue.family_index.compute_index = i;
                }
            }

            let found_graphics = queue.family_index.graphics_index >= 0;
            let found_present  = queue.family_index.present_index >= 0;
            let found_compute  = queue.family_index.compute_index >= 0;

            if ( found_graphics and found_compute and ( reference_surface == nullptr or found_present ) )
            {
                break;
            }
        }
    }

    // Release Vulkan resources used by this device wrapper
    void device::clean_up()
    {
        check_error_condition( return, vulkan_log_errors, is_initialized == false, "Destroying uninitialized Vulkan device wrapper" );

        tracked_subregion( vulkan_log_verbose, "Vulkan command pool destruction" )
        {
            vulkan_device.destroyCommandPool( command_pool.main() );
            vulkan_device.destroyCommandPool( command_pool.transfer() );
        }

        tracked_subregion( vulkan_log_verbose, "Vulkan device destruction" )
        {
            vulkan_device.destroy();
        }

        is_initialized = false;
    }

    void device::wait_for_idle() const
    {
        vulkan_device.waitIdle();
    }

/* -------------------------------------------------------------------------- */
/*                      Physical Device Selection Helpers                     */
/* -------------------------------------------------------------------------- */
#pragma region helpers

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

        let requires_present = surface != nullptr && *surface;
        let queue_support    = get_device_queue_support( device, surface );

        check_error_condition( return bad_device, vulkan_log_errors, queue_support.graphics == false, "Device (\1) doesn't support rendering", properties.deviceName );
        check_error_condition( return bad_device, vulkan_log_errors, requires_present and not queue_support.present, "Device (\1) doesn't support surface display", properties.deviceName );

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

#pragma endregion helpers

} // namespace rnjin::graphics::vulkan