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
        pass_member( target ),          //
        frame_number( 0 )               //
    {}
    renderer::~renderer() {}

    void renderer::define() {}
    void renderer::initialize() {}

    void renderer::before_update()
    {
        frame_number++;

        vulkan_log_verbose.print( "[\1] vulkan::renderer before update", frame_number );

        target.begin_frame();
        current_frame.command_buffer = target.get_current_command_buffer();
    }

    void renderer::update( entity_components& components )
    {
        vulkan_log_verbose.print( "[\1] vulkan::renderer begin update", frame_number );
        let& resources = components.readable<vulkan::internal_resources>();

        subregion // handle pipeline state change
        {
            current_frame.command_buffer.bindPipeline(
                vk::PipelineBindPoint::eGraphics,              // pipelineBindPoint
                resources.get_pipeline().get_vulkan_pipeline() // pipeline
            );
        }

        subregion // handle vertex buffer state change
        {
            // note: does this need to be bound multiple times, since all vertex buffers
            //       are allocated from the same actual buffer? Maybe just bind once
            //       and use an offer parameter in drawIndexed? Also needs to handle
            //       instanced rendering and such.
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

        subregion // handle index buffer state change
        {
            // Check at compile-time what the correct buffer type is based on the size of mesh::index
            let constexpr index_buffer_type = ( sizeof( mesh::index ) == sizeof( uint16 ) ) ? vk::IndexType::eUint16 : vk::IndexType::eUint32;

            current_frame.command_buffer.bindIndexBuffer(
                resources.get_indices().get_buffer(), // buffer
                resources.get_indices().get_offset(), // offset
                index_buffer_type                     // type
            );
        }

        subregion // handle descriptor set state change
        {
            current_frame.command_buffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,               // pipelineBindPoint,
                resources.get_pipeline().get_layout(),         // layout,
                0,                                              // firstSet,
                1,                                              // descriptorSetCount
                &resources.get_pipeline().get_descriptor_set(), // pDescriptorSets,
                0,                                              // dynamicOffsetCount
                nullptr                                         // pDynamicOffsets
            );
        }

        // Do final drawing operation
        current_frame.command_buffer.drawIndexed(
            resources.get_index_count(), // indexCount
            1,                           // instanceCount
            0,                           // firstIndex
            0,                           // vertexOffset
            0                            // firstInstance
        );
        vulkan_log_verbose.print( "[\1] vulkan::renderer finish update", frame_number );
    }

    void renderer::after_update()
    {
        vulkan_log_verbose.print( "[\1] vulkan::renderer after update", frame_number );
        target.end_frame();
    }
} // namespace rnjin::graphics::vulkan