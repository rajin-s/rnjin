/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "resource_database.hpp"
#include "vulkan_renderer_internal.hpp"


namespace rnjin::graphics::vulkan
{
    resource_database::resource_database( renderer_internal& parent ) : child_class( parent ) {}
    resource_database::~resource_database() {}

    /// Mesh data management ///
    using mesh_entry = resource_database::mesh_entry;

    let static invalid_mesh_data = mesh_entry();
    static const uint find_best_memory_type( vk::PhysicalDevice device, bitmask type_filter, vk::MemoryPropertyFlags target_properties );

    const mesh_entry& resource_database::get_mesh_data( const resource::id resource_id ) const
    {
        check_error_condition( return invalid_mesh_data, vulkan_log_errors, mesh_data.count( resource_id ) == 0, "Failed to find Vulkan mesh resource with id=\1", resource_id );
        return mesh_data.at( resource_id );
    }

    const mesh_entry& resource_database::store_mesh_data( const mesh& mesh_resource )
    {
        let mesh_id = mesh_resource.get_id();

        let has_data_for_mesh = mesh_data.count( mesh_id ) > 0;
        check_error_condition( return mesh_data.at( mesh_id ), vulkan_log_errors, has_data_for_mesh == true, "Vulkan data for mesh #\1 already exists", mesh_id );

        let& device = parent.device.vulkan_device;

        // Constant flags for creating buffers and allocating memory
        let buffer_sharing_mode = vk::SharingMode::eExclusive;
        let buffer_memory_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        vk::Buffer new_vertex_buffer;
        vk::DeviceMemory new_vertex_buffer_memory;

        vk::Buffer new_index_buffer;
        vk::DeviceMemory new_index_buffer_memory;

        tracked_subregion( vulkan_log_verbose, "Vulkan vertex buffer creation" )
        {
            // Get data from mesh
            let& mesh_vertices = mesh_resource.get_vertices();
            let buffer_size    = sizeof( mesh::vertex ) * mesh_vertices.size();

            // Create the buffer (no memory allocated yet)
            vk::BufferCreateInfo buffer_info(
                {},                                     // flags
                buffer_size,                            // size
                vk::BufferUsageFlagBits::eVertexBuffer, // usage
                buffer_sharing_mode                     // sharingMode
            );
            new_vertex_buffer = device.createBuffer( buffer_info );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not new_vertex_buffer, "Failed to create vertex buffer for mesh resource #\1", mesh_id );

            // Get memory information based on buffer requirements
            let buffer_memory_requirements = device.getBufferMemoryRequirements( new_vertex_buffer );
            let buffer_memory_type_index   = find_best_memory_type( parent.device.physical_device, buffer_memory_requirements.memoryTypeBits, buffer_memory_flags );

            // Allocate memory for the buffer
            vk::MemoryAllocateInfo buffer_allocation_info(
                buffer_memory_requirements.size, // allocationSize
                buffer_memory_type_index         // memoryTypeIndex
            );
            new_vertex_buffer_memory = device.allocateMemory( buffer_allocation_info );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not new_vertex_buffer_memory, "Failed to allocate vertex buffer memory for mesh resource #\1", mesh_resource.get_id() );
        }

        tracked_subregion( vulkan_log_verbose, "Vulkan index buffer creation" )
        {
            // Get data from mesh
            let& mesh_indices = mesh_resource.get_indices();
            let buffer_size   = sizeof( mesh::index ) * mesh_indices.size();

            // Create the buffer (no memory allocated yet)
            vk::BufferCreateInfo buffer_info(
                {},                                    // flags
                buffer_size,                           // size
                vk::BufferUsageFlagBits::eIndexBuffer, // usage
                buffer_sharing_mode                    // sharingMode
            );
            new_index_buffer = device.createBuffer( buffer_info );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not new_index_buffer, "Failed to create index buffer for mesh resource #\1", mesh_id );

            // Get memory information based on buffer requirements
            let buffer_memory_requirements = device.getBufferMemoryRequirements( new_index_buffer );
            let buffer_memory_type_index   = find_best_memory_type( parent.device.physical_device, buffer_memory_requirements.memoryTypeBits, buffer_memory_flags );

            // Allocate memory for the buffer
            vk::MemoryAllocateInfo buffer_allocation_info(
                buffer_memory_requirements.size, // allocationSize
                buffer_memory_type_index         // memoryTypeIndex
            );
            new_index_buffer_memory = device.allocateMemory( buffer_allocation_info );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not new_index_buffer_memory, "Failed to allocate index buffer memory for mesh resource #\1", mesh_id );
        }

        // Create and insert the new entry into the database
        let new_entry = mesh_entry{ new_vertex_buffer, new_vertex_buffer_memory, new_index_buffer, new_index_buffer_memory };
        mesh_data.insert_or_assign( mesh_id, new_entry );
        return mesh_data.at( mesh_id );
    }

    // Static helper function to find a suitable memory type (based on type_filter) that has all the required property flags (target_properties)
    const uint find_best_memory_type( vk::PhysicalDevice device, bitmask type_filter, vk::MemoryPropertyFlags target_properties )
    {
        let device_memory_properties = device.getMemoryProperties();

        // Go through all available memory types
        for ( uint i : range( device_memory_properties.memoryTypeCount ) )
        {
            let is_suitable_type = type_filter[i];
            if ( is_suitable_type )
            {
                // Check that the current memory type has all the target flags
                let device_memory_property_flags = device_memory_properties.memoryTypes[i].propertyFlags;
                let has_target_properties        = ( device_memory_property_flags & target_properties ) == target_properties;

                if ( has_target_properties )
                {
                    return i;
                }
            }
        }

        // No memory type was found that is suitable and had all the needed flags
        const bool failed_to_find_memory_type = true;
        check_error_condition( return 0, vulkan_log_errors, failed_to_find_memory_type == true, "Fauled to find a suitable memory type" );
    }
} // namespace rnjin::graphics::vulkan