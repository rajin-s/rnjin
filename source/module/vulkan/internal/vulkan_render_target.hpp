/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "vulkan_api_internal.hpp"
#include "render_view.hpp"

namespace rnjin::graphics::vulkan
{
    // Forward declarations
    class renderer_internal;

    class render_target : public child_class<const renderer_internal>
    {
        public: // methods
        render_target( const renderer_internal& parent );
        ~render_target();

        virtual void initialize() is_abstract;
        virtual void render( const render_view& view ) is_abstract;
        virtual void clean_up() is_abstract;

        protected: // command buffer interaction methods
        struct begin_frame_info
        {
            vk::RenderPass render_pass;
            vk::Framebuffer frame_buffer;
            uint2 image_size;

            bool clear;
            vk::ClearValue clear_value;
        };
        struct draw_call_info
        {
            vk::Buffer vertex_buffer;
            vk::Buffer index_buffer;
            vk::Pipeline pipeline;

            uint vertex_count;
            uint index_count;
        };

        // Prepare a command buffer for the start of a single frame
        void begin_frame( const begin_frame_info info, vk::CommandBuffer& command_buffer );

        // Draw something based on all the info in a draw_call_info struct
        void draw( const draw_call_info info, vk::CommandBuffer& command_buffer );

        // Prepare a command buffer for the end of a single frame
        void end_frame( vk::CommandBuffer& command_buffer );
    };
} // namespace rnjin::graphics::vulkan