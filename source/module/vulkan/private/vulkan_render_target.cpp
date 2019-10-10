/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_renderer_internal.hpp"
#include "vulkan_render_target.hpp"

namespace rnjin::graphics::vulkan
{
    render_target::render_target( const renderer_internal& parent ) : child_class( parent ) {}
    render_target::~render_target() {}

    void render_target::begin_frame( const begin_frame_info info, vk::CommandBuffer& command_buffer )
    {
        let begin_info = vk::CommandBufferBeginInfo(
            {},     // flags
            nullptr // pInheritanceInfo
        );

        command_buffer.begin( begin_info );

        let full_screen_area = vk::Rect2D(
            vk::Offset2D( 0, 0 ),                                // origin
            vk::Extent2D( info.image_size.x, info.image_size.y ) // size
        );
        let subpass_contents = vk::SubpassContents::eInline;

        let render_pass_info = vk::RenderPassBeginInfo(
            info.render_pass,   // renderPass
            info.frame_buffer,  // framebuffer
            full_screen_area,   // renderArea
            info.clear ? 1 : 0, // clearValueCount
            &info.clear_value   // pClearValues
        );

        command_buffer.beginRenderPass( render_pass_info, subpass_contents );

        let viewport = vk::Viewport( 0.0, 0.0, info.image_size.x, info.image_size.y );
        command_buffer.setViewport( 0, 1, &viewport );
        command_buffer.setScissor( 0, 1, &full_screen_area );
    }

    void render_target::draw( const draw_call_info info, vk::CommandBuffer& command_buffer )
    {
        command_buffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics, // pipelineBindPoint
            info.pipeline                     // pipeline
        );

        let vertex_buffer_binding = 0;
        let vertex_buffer_offset  = vk::DeviceSize( 0 );

        command_buffer.bindVertexBuffers(
            vertex_buffer_binding, // firstBinding
            1,                     // bindingCount
            &info.vertex_buffer,   // pBuffers
            &vertex_buffer_offset   // pOffsets
        );

        let index_buffer_offset = vk::DeviceSize( 0 );

        // Check at compile-time what the correct buffer type is based on the size of mesh::index
        let constexpr index_buffer_type = ( sizeof( mesh::index ) == sizeof( uint16 ) ) ? vk::IndexType::eUint16 : vk::IndexType::eUint32;

        command_buffer.bindIndexBuffer(
            info.index_buffer,   // buffer
            index_buffer_offset, // offset
            index_buffer_type    // type
        );

        command_buffer.drawIndexed(
            info.index_count, // indexCount
            1,                // instanceCount
            0,                // firstIndex
            0,                // vertexOffset
            0                 // firstInstance
        );
    }

    void render_target::end_frame( vk::CommandBuffer& command_buffer )
    {
        command_buffer.endRenderPass();
        command_buffer.end();
    }
} // namespace rnjin::graphics::vulkan
