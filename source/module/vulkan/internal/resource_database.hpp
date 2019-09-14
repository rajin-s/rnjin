/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <vulkan/vulkan.hpp>

#include "mesh.hpp"
// #include "vulkan_renderer_internal.hpp"


namespace rnjin::graphics::vulkan
{
    // forward declarations
    class renderer_internal;

    class resource_database : child_class<renderer_internal>
    {
        public: // methods
        resource_database( renderer_internal& parent );
        ~resource_database();

        public: // structures
        struct mesh_entry
        {
            vk::Buffer vertex_buffer;
            vk::DeviceMemory vertex_buffer_memory;

            vk::Buffer index_buffer;
            vk::DeviceMemory index_buffer_memory;
        };

        const mesh_entry& get_mesh_data( const resource::id resource_id ) const;
        const mesh_entry& store_mesh_data( const mesh& mesh_resource );
        void release_mesh_data( const mesh& mesh_resource );

        private: // members
        dictionary<resource::id, mesh_entry> mesh_data;
    };
} // namespace rnjin::graphics::vulkan