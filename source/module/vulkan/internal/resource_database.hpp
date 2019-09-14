/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <vulkan/vulkan.hpp>

#include "mesh.hpp"
#include "vulkan_renderer_internal.hpp"

namespace rnjin::graphics::vulkan
{
    class resource_database : child_class<renderer_internal>
    {
        public: // methods
        resource_database( renderer_internal& parent );
        ~resource_database();

        public: // structures
        group meshes
        {
            struct entry
            {
                entry( vk::Buffer vertex_buffer, vk::DeviceMemory vertex_buffer_memory );
                const vk::Buffer vertex_buffer;
                const vk::DeviceMemory vertex_buffer_memory;
            };

            const entry& get( const resource::id resource_id ) const;
            const entry& create( const mesh& mesh_resource );
            void release( const mesh& mesh_resource );

            private:
            dictionary<resource::id, entry> entries;
        }
        mesh_data;

        struct foo
        {
            foo( resource_database* c ) : c( c ) {}

            private:
            resource_database* c;
        } bar{ this };
    };
} // namespace rnjin::graphics::vulkan