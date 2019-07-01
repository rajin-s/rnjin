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
            // Create a Vulkan rendering surface attached to the given GLFW window
            void internal::surface::initialize( const instance& reference_instance, const window<GLFW>& target_window )
            {
                graphics_log.print( "Starting Vulkan surface initialization...", log_channel_vulkan );

                glfw_window = &target_window;
                if ( glfwCreateWindowSurface( reference_instance.get_vulkan_instance(), target_window.get_api_window(), nullptr, &vulkan_surface ) != VK_SUCCESS )
                {
                    throw std::runtime_error( "Failed to create GLFW Vulkan window surface!" );
                }

                graphics_log.print( "Finished Vulkan surface initialization", log_channel_vulkan );
            }

            // Destroy the Vulkan rendering surface
            void internal::surface::clean_up( const instance& reference_instance, const device& reference_device )
            {
                for ( uint i : range( 0, swap_chain_image_views.size() ) )
                {
                    vkDestroyImageView( reference_device.get_vulkan_device(), swap_chain_image_views[i], nullptr );
                    swap_chain_image_views[i] = VK_NULL_HANDLE;
                    swap_chain_images[i]      = VK_NULL_HANDLE;
                }
                graphics_log.print( "Finished Vulkan image view cleanup", log_channel_vulkan );

                vkDestroySwapchainKHR( reference_device.get_vulkan_device(), swap_chain, nullptr );
                {
                    swap_chain = VK_NULL_HANDLE;
                }
                graphics_log.print( "Finished Vulkan swapchain cleanup", log_channel_vulkan );

                vkDestroySurfaceKHR( reference_instance.get_vulkan_instance(), vulkan_surface, nullptr );
                {
                    vulkan_surface = VK_NULL_HANDLE;
                    glfw_window    = nullptr;
                }
                graphics_log.print( "Finished Vulkan surface cleanup", log_channel_vulkan );
            }

            // Get the actual Vulkan surface object
            const VkSurfaceKHR internal::surface::get_vulkan_surface() const
            {
                return vulkan_surface;
            }

            // Forward declaration of internal helper functions
            VkSurfaceFormatKHR get_best_swap_surface_format( const list<VkSurfaceFormatKHR>& available_formats );
            VkPresentModeKHR get_best_swap_present_mode( const list<VkPresentModeKHR>& available_present_modes );
            VkExtent2D get_best_swap_extent( const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t window_width, const uint32_t window_height );

            // Create a swap chain for this surface based on the capabilities of the given device
            // note: call after surface::initialize() and device::initialize()
            void internal::surface::create_swap_chain( const device& reference_device )
            {
                device::swap_chain_support_details details = reference_device.get_swap_chain_support( *this );

                VkSurfaceFormatKHR format     = get_best_swap_surface_format( details.formats );
                VkPresentModeKHR present_mode = get_best_swap_present_mode( details.present_modes );

                const int2 window_size = glfw_window->get_size();
                VkExtent2D extent      = get_best_swap_extent( details.capabilities, window_size.x, window_size.y );

                uint image_count = details.capabilities.minImageCount + 1;
                if ( details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount )
                {
                    image_count = details.capabilities.maxImageCount;
                }

                VkSwapchainCreateInfoKHR create_info = {};
                {
                    create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                    create_info.surface          = vulkan_surface;
                    create_info.presentMode      = present_mode;
                    create_info.minImageCount    = image_count;
                    create_info.imageFormat      = format.format;
                    create_info.imageColorSpace  = format.colorSpace;
                    create_info.imageExtent      = extent;
                    create_info.imageArrayLayers = 1;
                    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                    create_info.preTransform     = details.capabilities.currentTransform;
                    create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                    create_info.clipped          = VK_TRUE;
                    create_info.oldSwapchain     = VK_NULL_HANDLE;
                }

                device::queue_family_indices indices = reference_device.get_queue_indices();
                uint queue_indices[]                 = { indices.graphics, indices.present }; // todo: these better not be negative

                if ( indices.graphics != indices.present )
                {
                    create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                    create_info.queueFamilyIndexCount = 2;
                    create_info.pQueueFamilyIndices   = queue_indices;
                }
                else
                {
                    create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                    create_info.queueFamilyIndexCount = 0;
                    create_info.pQueueFamilyIndices   = nullptr;
                }

                if ( vkCreateSwapchainKHR( reference_device.get_vulkan_device(), &create_info, nullptr, &swap_chain ) != VK_SUCCESS )
                {
                    throw std::runtime_error( "Failed to create Vulkan swap chain!" );
                }

                // Get swap chain images
                vkGetSwapchainImagesKHR( reference_device.get_vulkan_device(), swap_chain, &image_count, nullptr );
                swap_chain_images.resize( image_count );
                vkGetSwapchainImagesKHR( reference_device.get_vulkan_device(), swap_chain, &image_count, swap_chain_images.data() );

                // Save swap chain image details
                swap_chain_image_format = format.format;
                swap_chain_extent       = extent;

                // Create image views to render swap chain images
                create_image_views( reference_device );

                graphics_log.print( "Finished Vulkan swapchain creation" );
            }

            // Create image views from existing swap chain images
            void internal::surface::create_image_views( const device& reference_device )
            {
                const uint image_count = swap_chain_images.size();
                swap_chain_image_views.resize( image_count );

                for ( uint i : range( 0, image_count ) )
                {
                    VkImageViewCreateInfo create_info = {};
                    {
                        create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                        create_info.image    = swap_chain_images[i];
                        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                        create_info.format   = swap_chain_image_format;

                        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                        create_info.subresourceRange.levelCount     = 1;
                        create_info.subresourceRange.baseMipLevel   = 0;
                        create_info.subresourceRange.layerCount     = 1;
                        create_info.subresourceRange.baseArrayLayer = 0;
                    }

                    if ( vkCreateImageView( reference_device.get_vulkan_device(), &create_info, nullptr, &swap_chain_image_views[i] ) != VK_SUCCESS )
                    {
                        throw std::runtime_error( "Failed to create Vulkan image view!" );
                    }
                }
            }

            /// Internal helper functions ///

            VkSurfaceFormatKHR get_best_swap_surface_format( const list<VkSurfaceFormatKHR>& available_formats )
            {
                // Find a common easy-to-use format
                foreach ( format : available_formats )
                {
                    if ( format.format == VK_FORMAT_B8G8R8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
                    {
                        return format;
                    }
                }

                // Default to first format
                // note: could do more logic to rank formats
                return available_formats[0];
            }

            VkPresentModeKHR get_best_swap_present_mode( const list<VkPresentModeKHR>& available_present_modes )
            {
                // fifo is a mostly guaranteed default, might need to use immediate
                VkPresentModeKHR fallback = VK_PRESENT_MODE_FIFO_KHR;

                // See if any of the available present modes are triple buffered, or if immediate is needed
                foreach ( present_mode : available_present_modes )
                {
                    if ( present_mode == VK_PRESENT_MODE_MAILBOX_KHR )
                    {
                        return present_mode;
                    }
                    else if ( present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR )
                    {
                        fallback = present_mode;
                    }
                }

                return fallback;
            }

            VkExtent2D get_best_swap_extent( const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t window_width, const uint32_t window_height )
            {
                if ( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() )
                {
                    return capabilities.currentExtent;
                }
                else
                {
                    VkExtent2D extent = { clamp( window_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width ),
                                          clamp( window_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height ) };
                    return extent;
                }
            }
        } // namespace vulkan

    } // namespace graphics
} // namespace rnjin