/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <vulkan/vulkan.hpp>

#include "shader.hpp"
#include "vulkan_renderer.hpp"
#include "vulkan_api_internal.hpp"

namespace rnjin::graphics::vulkan
{
    // forward declarations
    class window_surface;

    /// Private Renderer API (used in vulkan_renderer.cpp) ///
    class renderer_internal
    {
        public: // methods
        renderer_internal( api& api_instance );
        ~renderer_internal();

        void initialize();
        void clean_up();
        void add_window_target( window<GLFW>& target );

        void render( const render_view& view );

        private: // methods
        void create_device( const vk::SurfaceKHR* reference_surface );
        void destroy_device();

        private: // accessors
        let& get_vulkan_instance get_value( api_instance.internal->instance.vulkan_instance );

        private: // members
        api& api_instance;

        group device_info
        {
            bool initialized;
            vk::PhysicalDevice physical_device;
            vk::Device vulkan_device;
            vk::CommandPool command_pool;

            void find_queue_family_indices( const vk::SurfaceKHR* reference_surface );

            group
            {
                vk::Queue graphics;
                vk::Queue present;
                vk::Queue compute;

                group
                {
                    int graphics;
                    int present;
                    int compute;
                }
                family_index;
            }
            queue;
        }
        device;

        list<window_surface> window_targets;
        friend class window_surface;
    };

    // Helper functions and constant data
    extern const list<const char*> required_device_extensions;
    extern const list<const char*> device_present_extensions;

    const vk::SurfaceFormatKHR get_best_surface_format( const list<vk::SurfaceFormatKHR>& available_formats );
    const vk::PresentModeKHR get_best_present_mode( const list<vk::PresentModeKHR>& available_present_modes );
    const uint2 get_best_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities, const uint2 window_size );
    const int get_device_suitability( const vk::PhysicalDevice device, const vk::SurfaceKHR* reference_surface, const bool require_validation );
} // namespace rnjin::graphics::vulkan