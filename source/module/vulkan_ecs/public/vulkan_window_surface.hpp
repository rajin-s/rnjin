/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include <vulkan/vulkan.hpp>

#include "vulkan_device.hpp"
#include "graphics/windowing.h"

namespace rnjin::graphics::vulkan
{
    class window_surface
    {
        public: // methods
        window_surface( const device& device_instance );
        ~window_surface();

        // note: order of events is
        //       1. device created
        //       2. window surface created
        //       3. vk::Surface created
        //       4. device initialized
        //       5. window surface initialized
        void create_surface( window<GLFW>& target );

        void initialize();
        void clean_up();

        void begin_frame();
        void end_frame();

        public: // accessors
        let& get_vulkan_surface get_value( vulkan_surface );
        let& get_render_pass get_value( render_pass );

        let& get_current_command_buffer get_value( swapchain.get_current().command_buffer );

        private: // methods
        void destroy_surface();

        void create_swapchain();
        void destroy_swapchain();

        void create_depth_buffer();
        void destroy_depth_buffer();

        void create_render_pass();
        void destroy_render_pass();

        void create_frame_buffers();
        void destroy_frame_buffers();

        void create_command_buffers();

        void initialize_synchronization();
        void destroy_synchronization();

        void handle_out_of_date_swapchain();
        void handle_resize( const uint2 new_size );

        private: // members
        const device& device_instance;
        uint2 window_size;

        const window<GLFW>* window_pointer;
        vk::SurfaceKHR vulkan_surface;
        vk::RenderPass render_pass;

        group
        {
            vk::Image image;
            vk::DeviceMemory device_memory;
            vk::ImageView image_view;
        } depth_buffer;

        group swapchain_info
        {
            vk::SwapchainKHR vulkan_swapchain;
            vk::SurfaceFormatKHR format;
            vk::PresentModeKHR present_mode;

            uint2 image_size;
            uint image_count;

            group
            {
                const uint max_frames_in_flight = 2;
                uint current_frame;

                struct frame_info
                {
                    vk::Semaphore image_available;
                    vk::Semaphore render_finished;
                    vk::Fence in_flight;
                };

                const frame_info& get_current_frame_info() const
                {
                    return frames[current_frame];
                }

                void advance_frame() nonconst
                {
                    current_frame = ( current_frame + 1 ) % max_frames_in_flight;
                }

                list<frame_info> frames;
            }
            synchronization;

            struct element
            {
                vk::Image image;
                vk::ImageView image_view;
                vk::Framebuffer frame_buffer;
                vk::CommandBuffer command_buffer;
            };

            list<element> elements;
            uint current_element_index;

            const element& get_current get_value( elements[current_element_index] );
        }
        swapchain;
    };
} // namespace rnjin::graphics::vulkan