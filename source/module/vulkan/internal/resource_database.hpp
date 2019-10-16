/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <vulkan/vulkan.hpp>

#include "mesh.hpp"
#include "material.hpp"
#include "event.hpp"

namespace rnjin::graphics::vulkan
{
    // forward declarations
    class renderer_internal;

    class resource_database : child_class<const renderer_internal>, event_receiver
    {
        public: // methods
        resource_database( const renderer_internal& parent );
        ~resource_database();

        void initialize();
        void clean_up();

        private: // methods
        void on_mesh_loaded( const mesh& mesh_resource );
        void on_mesh_destroyed( const mesh& mesh_resource );
        void on_material_loaded( const material& material_resource );
        void on_material_destroyed( const material& material_resource );

        struct create_buffer_result
        {
            create_buffer_result() : valid( false ) {}
            create_buffer_result( vk::Buffer buffer, vk::DeviceMemory buffer_memory ) : pass_member( buffer ), pass_member( buffer_memory ), valid( true ) {}

            vk::Buffer buffer;
            vk::DeviceMemory buffer_memory;
            const bool valid;
        };

        create_buffer_result create_buffer( usize size, vk::BufferUsageFlags usage_flags, vk::MemoryPropertyFlags memory_property_flags );
        void copy_buffer( vk::Buffer source, vk::Buffer destination, usize size );

        public: // structures
        struct mesh_entry
        {
            bool is_valid;

            vk::Buffer vertex_buffer;
            vk::DeviceMemory vertex_buffer_memory;
            uint vertex_count;

            vk::Buffer index_buffer;
            vk::DeviceMemory index_buffer_memory;
            uint index_count;

            mesh_entry();
            mesh_entry( vk::Buffer vertex_buffer, vk::DeviceMemory vertex_buffer_memory, uint vertex_buffer_count, vk::Buffer index_buffer, vk::DeviceMemory index_buffer_memory, uint index_buffer_count );
        };

        const mesh_entry& get_mesh_data( const resource::id resource_id ) const;
        const mesh_entry& store_mesh_data( const mesh& mesh_resource );
        void release_mesh_data( const resource::id resource_id );

        struct material_entry
        {
            bool is_valid;

            vk::PipelineLayout pipeline_layout;
            vk::Pipeline pipeline;

            material_entry();
            material_entry( vk::PipelineLayout pipeline_layout, vk::Pipeline pipeline );
        };

        const material_entry& get_material_data( const resource::id resource_id ) const;
        const material_entry& store_material_data( const material& material_resource );
        void release_material_data( const resource::id resource_id );

        private: // members
        dictionary<resource::id, mesh_entry> mesh_data;
        dictionary<resource::id, material_entry> material_data;
        vk::PipelineCache pipeline_cache;
    };
} // namespace rnjin::graphics::vulkan