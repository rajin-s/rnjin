/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            const list<const char*> required_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

            // Pick a suitable physical device and create a Vulkan logical device
            void internal::device::initialize( const instance& reference_instance, const surface& reference_surface, const validation& reference_validation )
            {
                graphics_log.print( "Starting Vulkan device initialization...", log_channel_vulkan );

                graphics_log.print( "Picking best physical device...", log_channel_vulkan );
                {
                    uint32_t physical_device_count = 0;
                    vkEnumeratePhysicalDevices( reference_instance.get_vulkan_instance(), &physical_device_count, nullptr );
                    if ( physical_device_count == 0 )
                    {
                        throw std::runtime_error( "Failed to find GPU with Vulkan support!" );
                    }
                    list<VkPhysicalDevice> devices( physical_device_count );
                    vkEnumeratePhysicalDevices( reference_instance.get_vulkan_instance(), &physical_device_count, devices.data() );

                    graphics_log.printf( "Found \1 Vulkan-compatible devices", { s( physical_device_count ) }, log_channel_vulkan );

                    int max_suitability          = -1;
                    VkPhysicalDevice best_device = VK_NULL_HANDLE;

                    foreach ( device : devices )
                    {
                        int suitability = get_device_suitability( device, reference_surface.get_vulkan_surface() );
                        if ( suitability > max_suitability )
                        {
                            best_device     = device;
                            max_suitability = suitability;
                        }
                    }

                    if ( best_device == VK_NULL_HANDLE )
                    {
                        throw std::runtime_error( "Failed to find a suitable GPU!" );
                    }

                    graphics_log.print( "Successfully picked Vulkan physical device", log_channel_vulkan );
                    physical = best_device;
                }

                graphics_log.print( "Creating logical device...", log_channel_vulkan );
                {
                    // Get queue creation info
                    static const float queue_priority = 1.0;
                    list<VkDeviceQueueCreateInfo> queue_create_infos;

                    queue_family_indices queue_families = get_supported_queue_families_for_device( physical, reference_surface.get_vulkan_surface() );
                    set<int> unique_queue_families      = { queue_families.graphics, queue_families.compute, queue_families.present };
                    foreach ( family : unique_queue_families )
                    {
                        if ( family >= 0 )
                        {
                            VkDeviceQueueCreateInfo queue_create_info = {};
                            {
                                queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                                queue_create_info.queueFamilyIndex = family;
                                queue_create_info.queueCount       = 1;
                                queue_create_info.pQueuePriorities = &queue_priority;
                            }

                            queue_create_infos.push_back( queue_create_info );
                        }
                        else
                        {
                            graphics_log.printf( "Unsupported queue family: \1", { s( family ) }, log_channel_vulkan );
                        }
                    }

                    // Specify required device features (none for now)
                    VkPhysicalDeviceFeatures device_features = {};

                    // Create logical device
                    VkDeviceCreateInfo create_info = {};
                    {
                        create_info.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                        create_info.queueCreateInfoCount = queue_create_infos.size();
                        create_info.pQueueCreateInfos    = queue_create_infos.data();
                        create_info.pEnabledFeatures     = &device_features;
                    }

                    // Specify validation layers to use (only needed to support older Vulkan implementations)
                    list<const char*> validation_layers;
                    if ( reference_validation.is_enabled() )
                    {
                        list<string> validation_layer_names = reference_validation.get_supported_target_layers();
                        foreach ( name : validation_layer_names )
                        {
                            validation_layers.push_back( name.c_str() );
                        }

                        create_info.enabledLayerCount   = validation_layers.size();
                        create_info.ppEnabledLayerNames = validation_layers.data();
                    }
                    else
                    {
                        create_info.enabledLayerCount = 0;
                    }

                    // Specify device-specific extensions to use
                    create_info.enabledExtensionCount   = (uint)required_device_extensions.size();
                    create_info.ppEnabledExtensionNames = required_device_extensions.data();

                    // Actually create the device and assign it to device::logical_device if successful
                    if ( vkCreateDevice( physical, &create_info, nullptr, &vulkan_device ) != VK_SUCCESS )
                    {
                        throw std::runtime_error( "Failed to create Vulkan logical device!" );
                    }

                    // Save queue indices
                    queue.indices.graphics = queue_families.graphics;
                    queue.indices.present  = queue_families.present;
                    queue.indices.compute  = queue_families.compute;

                    // Get actual queue handles and assign to device::queue.**
                    if ( queue_families.graphics >= 0 )
                    {
                        vkGetDeviceQueue( vulkan_device, queue_families.graphics, 0, &queue.graphics );
                        graphics_log.printf( "Vulkan graphics queue: \1", { s( queue_families.graphics ) }, log_channel_vulkan );
                    }
                    if ( queue_families.compute >= 0 )
                    {
                        vkGetDeviceQueue( vulkan_device, queue_families.compute, 0, &queue.compute );
                        graphics_log.printf( "Vulkan compute queue: \1", { s( queue_families.compute ) }, log_channel_vulkan );
                    }
                    if ( queue_families.present >= 0 )
                    {
                        vkGetDeviceQueue( vulkan_device, queue_families.present, 0, &queue.present );
                        graphics_log.printf( "Vulkan present queue: \1", { s( queue_families.present ) }, log_channel_vulkan );
                    }
                }

                graphics_log.print( "Finished Vulkan device initialization", log_channel_vulkan );
            }

            // Destory the created device and reset handles
            void internal::device::clean_up()
            {
                vkDestroyDevice( vulkan_device, nullptr );
                {
                    vulkan_device = VK_NULL_HANDLE;
                    physical      = VK_NULL_HANDLE;

                    queue.graphics = VK_NULL_HANDLE;
                    queue.present  = VK_NULL_HANDLE;
                    queue.compute  = VK_NULL_HANDLE;
                }
                graphics_log.print( "Finished Vulkan device cleanup", log_channel_vulkan );
            }

            // Get the actual Vulkan logical device
            const VkDevice internal::device::get_vulkan_device() const
            {
                return vulkan_device;
            }

            // Get the details of how the selected physical device supports swapchains for the given surface
            const internal::device::swap_chain_support_details internal::device::get_swap_chain_support( const surface& reference_surface ) const
            {
                return get_swap_chain_support_details_for_device( physical, reference_surface.get_vulkan_surface() );
            }

            // Get the queue family indices on the selected physical device
            const internal::device::queue_family_indices internal::device::get_queue_indices() const
            {
                return queue.indices;
            }

            /// Internal helper functions ///

            // Get the details of how a device can interact with the given surface
            const internal::device::swap_chain_support_details internal::device::get_swap_chain_support_details_for_device( VkPhysicalDevice device, VkSurfaceKHR surface )
            {
                swap_chain_support_details details;

                vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &details.capabilities );

                // Get presentation formats
                uint32_t format_count;
                vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &format_count, nullptr );
                if ( format_count > 0 )
                {
                    details.formats.resize( format_count );
                    vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &format_count, details.formats.data() );
                }

                // Get presentation modes
                uint32_t present_mode_count;
                vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &present_mode_count, nullptr );
                if ( present_mode_count > 0 )
                {
                    details.present_modes.resize( present_mode_count );
                    vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &present_mode_count, details.present_modes.data() );
                }

                return details;
            }

            bool device_supports_extensions( VkPhysicalDevice device );

            // Get a score representing how suitable a given physical device is (higher is better)
            // The most sutable device should be chosen to ensure best performance.
            // note: scoring is mostly arbitrary right now
            const int internal::device::get_device_suitability( const VkPhysicalDevice device, const VkSurfaceKHR surface )
            {
                int score = 0;

                // Check device properties and features
                VkPhysicalDeviceProperties device_properties;
                VkPhysicalDeviceFeatures device_features;

                vkGetPhysicalDeviceProperties( device, &device_properties );
                vkGetPhysicalDeviceFeatures( device, &device_features );

                if ( device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) score += 1000;
                if ( device_features.geometryShader ) score += 200;
                if ( device_features.tessellationShader ) score += 200;

                queue_family_indices queue_families = get_supported_queue_families_for_device( device, surface );
                if ( queue_families.graphics >= 0 )
                {
                    score += 10000;
                }
                if ( queue_families.compute >= 0 )
                {
                    score += 100;
                }

                const bool supports_extensions = device_supports_extensions( device );

                if ( supports_extensions )
                {
                    // Check that the device supports swap chains for the given surface
                    swap_chain_support_details swap_chain_support = get_swap_chain_support_details_for_device( device, surface );
                    if ( swap_chain_support.formats.empty() || swap_chain_support.present_modes.empty() )
                    {
                        score = -1;
                    }
                }
                else
                {
                    score = -1;
                }

                graphics_log << log::use_channel( log_channel_vulkan );
                graphics_log << log::indent() << string( device_properties.deviceName ) << " (vendor:" << s( device_properties.vendorID ) << ") score: " << s( score ) << log::line();
                return score;
            }

            // Check what queue families are supported, and get their indices from the given device.
            // Also check if each queue family supports presentation to the given surface
            const internal::device::queue_family_indices internal::device::get_supported_queue_families_for_device( const VkPhysicalDevice device, const VkSurfaceKHR surface )
            {
                uint queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties( device, &queue_family_count, nullptr );
                list<VkQueueFamilyProperties> queue_families( queue_family_count );
                vkGetPhysicalDeviceQueueFamilyProperties( device, &queue_family_count, queue_families.data() );

                queue_family_indices indices;

                uint i = 0;
                foreach ( queue_family : queue_families )
                {
                    if ( queue_family.queueCount > 0 )
                    {
                        if ( indices.graphics < 0 && ( queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT ) )
                        {
                            indices.graphics = i;
                        }
                        else if ( queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT )
                        {
                            indices.compute = i;
                        }

                        VkBool32 present_supported = false;
                        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &present_supported );

                        if ( indices.present < 0 && present_supported )
                        {
                            indices.present = i;
                        }

                        if ( indices.is_complete() )
                        {
                            break;
                        }
                    }

                    i += 1;
                }
                return indices;
            }

            // Check if the device supports all the required device-level extensions
            bool device_supports_extensions( VkPhysicalDevice device )
            {
                // Get the supported extensions for the given device
                uint32_t extension_count;
                vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count, nullptr );

                list<VkExtensionProperties> available_extensions( extension_count );
                vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count, available_extensions.data() );

                // Check that all required extensions are present in the available extensions
                set<string> unsupported_extensions( required_device_extensions.begin(), required_device_extensions.end() );
                foreach ( extension : available_extensions )
                {
                    unsupported_extensions.erase( extension.extensionName );
                }

                return unsupported_extensions.empty();
            }
        } // namespace vulkan
    }     // namespace graphics
} // namespace rnjin