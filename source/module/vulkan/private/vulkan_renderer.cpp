/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_renderer.hpp"

namespace rnjin::graphics::vulkan
{
    renderer::renderer( const device& device_instance, window_surface& target )
      : pass_member( device_instance ), //
        pass_member( target )           //
    {}
    renderer::~renderer() {}

    void renderer::define() {}
    void renderer::initialize() {}

    void renderer::before_update()
    {
        vulkan_log_verbose.print( "vulkan::renderer before update" );
        target.begin_frame();
        current_frame.command_buffer = target.get_current_command_buffer();
    }

    void renderer::update( entity_components& components )
    {
        vulkan_log_verbose.print( "vulkan::renderer begin update" );
        let& resources = components.readable<vulkan::internal_resources>();

        subregion
        {
            current_frame.command_buffer.bindPipeline(
                vk::PipelineBindPoint::eGraphics,              // pipelineBindPoint
                resources.get_pipeline().get_vulkan_pipeline() // pipeline
            );
        }

        subregion
        {
            let vertex_buffer_binding = 0;
            let vertex_buffer         = resources.get_vertices().get_buffer();
            let vertex_buffer_offset  = resources.get_vertices().get_offset();

            current_frame.command_buffer.bindVertexBuffers(
                vertex_buffer_binding, // firstBinding
                1,                     // bindingCount
                &vertex_buffer,        // pBuffers
                &vertex_buffer_offset  // pOffsets
            );
        }

        subregion
        {
            // Check at compile-time what the correct buffer type is based on the size of mesh::index
            let constexpr index_buffer_type = ( sizeof( mesh::index ) == sizeof( uint16 ) ) ? vk::IndexType::eUint16 : vk::IndexType::eUint32;

            current_frame.command_buffer.bindIndexBuffer(
                resources.get_indices().get_buffer(), // buffer
                resources.get_indices().get_offset(), // offset
                index_buffer_type                     // type
            );
        }

        current_frame.command_buffer.drawIndexed(
            resources.get_index_count(), // indexCount
            1,                           // instanceCount
            0,                           // firstIndex
            0,                           // vertexOffset
            0                            // firstInstance
        );
        vulkan_log_verbose.print( "vulkan::renderer finish update" );
    }

    void renderer::after_update()
    {
        vulkan_log_verbose.print( "vulkan::renderer after update" );
        target.end_frame();
    }
} // namespace rnjin::graphics::vulkan