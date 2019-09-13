/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "vulkan_renderer_internal.hpp"

namespace rnjin::graphics::vulkan
{
    class window_surface : child_class<const renderer_internal>
    {
        public: // methods
        window_surface( const renderer_internal& parent );
        ~window_surface();

        void create_surface( window<GLFW>& reference_window );
        void destroy_surface();

        void create_swapchain();
        void destroy_swapchain();

        void create_render_pass();
        void destroy_render_pass();

        void create_frame_buffers();
        void destroy_frame_buffers();

        void create_command_buffers();

        void initialize_synchronization();
        void destroy_synchronization();

        uint create_pipeline( const shader& vertex_shader, const shader& fragment_shader );
        void destroy_pipelines();

        void render();

        void handle_out_of_date_swapchain();
        void handle_resize( const uint2 new_size );

        public: // accessors
        let& get_vulkan_surface get_value( vulkan_surface );

        private: // members
        uint2 window_size;

        const window<GLFW>* window_pointer;
        vk::SurfaceKHR vulkan_surface;

        vk::RenderPass render_pass;

        struct pipeline_info
        {
            vk::PipelineLayout layout;
            vk::Pipeline vulkan_pipeline;
        };
        list<pipeline_info> pipelines;

        group
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
        }
        swapchain;
    };
} // namespace rnjin::graphics::vulkan