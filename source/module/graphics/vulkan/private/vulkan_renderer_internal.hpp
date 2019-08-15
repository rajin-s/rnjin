/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan_api_internal.hpp"
#include "vulkan_renderer.hpp"

namespace rnjin
{
    namespace graphics
    {
        /// Private Renderer API (used in vulkan_renderer.cpp) ///
        class renderer<vulkan>::internal
        {
            public: // methods
            internal( vulkan& api );
            ~internal();

            void initialize();
            void clean_up();
            void add_window_target( window<GLFW>& target );

            void render( const render_view& view );

            private: // methods
            void create_device( const vk::SurfaceKHR* reference_surface );
            void destroy_device();

            private: // accessors
            let& get_vulkan_instance get_value( api._internal->instance.vulkan_instance );

            private: // members
            vulkan& api;

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

            class window_surface;
            list<window_surface> window_targets;

            class window_surface : child_class<const internal>
            {
                public: // methods
                window_surface( const internal& parent );
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
        };

        // Helper functions and constant data
        extern const list<const char*> required_device_extensions;
        extern const list<const char*> device_present_extensions;

        const vk::SurfaceFormatKHR get_best_surface_format( const list<vk::SurfaceFormatKHR>& available_formats );
        const vk::PresentModeKHR get_best_present_mode( const list<vk::PresentModeKHR>& available_present_modes );
        const uint2 get_best_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities, const uint2 window_size );
        const int get_device_suitability( const vk::PhysicalDevice device, const vk::SurfaceKHR* reference_surface, const bool require_validation );

    } // namespace graphics
} // namespace rnjin