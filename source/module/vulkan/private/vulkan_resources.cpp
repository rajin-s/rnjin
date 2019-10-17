/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_resources.hpp"

namespace rnjin::graphics::vulkan
{
    /* -------------------------------------------------------------------------- */
    /*                              Buffer Allocation                             */
    /* -------------------------------------------------------------------------- */

    buffer_allocation::buffer_allocation() : offset( 0 ), size( 0 ), buffer( nullptr ) {}
    buffer_allocation::buffer_allocation( vk::DeviceSize offset, vk::DeviceSize size, vk::Buffer* buffer )
      : pass_member( offset ), //
        pass_member( size ),   //
        pass_member( buffer )  //
    {}


    /* -------------------------------------------------------------------------- */
    /*                              Buffer Allocator                              */
    /* -------------------------------------------------------------------------- */

    uint find_best_memory_type( vk::PhysicalDevice device, bitmask type_filter, vk::MemoryPropertyFlags target_properties );

    buffer_allocator::buffer_allocator( const device& device_instance, vk::BufferUsageFlags usage_flags, vk::MemoryPropertyFlags memory_property_flags )
      : pass_member( device_instance ),       //
        pass_member( usage_flags ),           //
        pass_member( memory_property_flags ), //
        size( 0 ),                      //
        entry_block( 0, 0, nullptr, nullptr ) //
    {}

    buffer_allocator::~buffer_allocator()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan buffer allocator destruction" );

        let& vulkan_device = device_instance.get_vulkan_device();

        // Make sure no operations are in-progress before freeing buffers and memory
        vulkan_device.waitIdle();

        // Make sure the buffer was actually created
        if ( buffer )
        {
            vulkan_device.destroyBuffer( buffer );
        }

        // Make sure the memory was actually allocated
        if ( memory )
        {
            vulkan_device.freeMemory( memory );
        }

        // Free memory used by linked list of free blocks
        for ( block* current_block = entry_block.next; current_block != nullptr; )
        {
            block* next_block = current_block->next;
            delete current_block;
            current_block = next_block;
        }
    }

    void buffer_allocator::initialize( vk::DeviceSize total_size )
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan buffer allocator initialization" );

        size = total_size;

        let& vulkan_device = device_instance.get_vulkan_device();

        let buffer_sharing_mode = vk::SharingMode::eExclusive;
        vk::BufferCreateInfo buffer_info(
            {},                 // flags
            size,               // size
            usage_flags,        // usage
            buffer_sharing_mode // sharingMode
        );

        // Create the buffer (no memory allocated yet)
        buffer = device_instance.get_vulkan_device().createBuffer( buffer_info );

        // Get memory information based on buffer requirements
        let buffer_memory_requirements = vulkan_device.getBufferMemoryRequirements( buffer );
        let buffer_memory_type_index   = find_best_memory_type( device_instance.get_physical_device(), buffer_memory_requirements.memoryTypeBits, memory_property_flags );

        // Allocate memory for the buffer
        vk::MemoryAllocateInfo buffer_allocation_info(
            buffer_memory_requirements.size, // allocationSize
            buffer_memory_type_index         // memoryTypeIndex
        );
        memory = vulkan_device.allocateMemory( buffer_allocation_info );
        check_error_condition( return, vulkan_log_errors, not memory, "Failed to allocate device memory for Vulkan buffer object (size=\1)", size );

        vulkan_device.bindBufferMemory(
            buffer, // buffer
            memory, // memory
            0       // memoryOffset
        );

        // Initialize the first block of free memory
        block* first_block = new block( total_size, 0, &entry_block, nullptr );

        subregion
        {
            // The entry block will never be allocated to, since its size is set to 0
            entry_block.offset   = 0;
            entry_block.size     = 0;
            entry_block.previous = nullptr;
            entry_block.next     = first_block;
        }
    }

    // Reserve a portion of the whole buffer to use for an allocation of the given size
    // note: uses a first-fit allocation strategy
    buffer_allocation buffer_allocator::allocate( vk::DeviceSize size )
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan buffer_allocator allocation" );

        check_error_condition( return buffer_allocation(), vulkan_log_errors, size == 0, "Tried to allocate 0 bytes of GPU memory" );

        block* destination_block = nullptr;
        for ( block* current_block = &entry_block; current_block != nullptr; current_block = current_block->next )
        {
            if ( current_block->size >= size )
            {
                // Break once the first block that can fit the request is found
                destination_block = current_block;
                break;
            }
        }

        check_error_condition( return buffer_allocation(), vulkan_log_errors, destination_block == nullptr, "Failed to allocate GPU memory for a request of size \1", size );

        // The destination block size matches the request exactly, so just get rid of it
        if ( destination_block->size == size )
        {
            // note: previous is guaranteed to exist, as the entry block (the only block without a previous entry)
            //       has 0 size and will thus never be selected for allocation
            destination_block->previous->next = destination_block->next;
            return buffer_allocation( destination_block->offset, size, &buffer );
            delete destination_block;
        }
        else
        {
            // The destination block size is larger than the request, so reserve the first part for the allocation and adjust values
            let result = buffer_allocation( destination_block->offset, size, &buffer );
            destination_block->size -= size;
            destination_block->offset += size;
            return result;
        }
    }

    // Release a portion of the whole buffer that was previously allocated, coalescing the list of free blocks as needed
    // note: should only be called for buffer_allocation objects that this instance allocated; this isn't enforced
    //       (maybe we should store an 'owner' pointer in the allocation and check that here?)
    void buffer_allocator::free( buffer_allocation& allocation )
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan buffer_allocator de-allocation" );

        block* previous_block = &entry_block;
        block* next_block     = nullptr;

        for ( block* current_block = &entry_block; current_block != nullptr; current_block = current_block->next )
        {
            if ( current_block->offset < allocation.offset )
            {
                previous_block = current_block;
            }
            else if ( current_block->offset > allocation.offset )
            {
                // Break once the first block beyond the allocation's offset is found
                // note: technically, the next block could only start at allocation.offset + allocation.size,
                //       but no other blocks could be allocated in this space, so this check is sufficient
                next_block = current_block;
                break;
            }
        }

        // Insert new free block and coalesce as needed

        if ( next_block == nullptr )
        {
            // No next block was found, so the allocation will be returned to the end of the list

            if ( previous_block->offset + previous_block->size == allocation.offset )
            {
                // The start of this block is the same as the end of the previous block, so just add to its size
                previous_block->size += allocation.size;
            }
            else
            {
                // Create a new block and link it onto the end of the list
                block* new_block     = new block( allocation.offset, allocation.size, previous_block, nullptr );
                previous_block->next = new_block;
            }
        }
        else if ( previous_block == &entry_block )
        {
            // This block is being returned to the front of the list
            // note: we don't want to coalesce with the entry block

            if ( allocation.offset + allocation.size == next_block->offset )
            {
                // The allocation is adjacent to the next free block, so just update its size of offset
                next_block->offset -= allocation.size;
                next_block->size += allocation.size;
            }
            else
            {
                // There is space between the allocation and the next free block, so create a new block
                // and link it into the list as appropriate

                block* new_block     = new block( allocation.size, allocation.offset, previous_block, next_block );
                previous_block->next = new_block;
                next_block->previous = new_block;
            }
        }
        else
        {
            // There is a previous (non-entry) and next block

            if ( previous_block->offset + previous_block->size + allocation.size == next_block->offset )
            {
                // This block perfectly fills the space between the previous and next blocks, so go ahead and merge the two
                // note: updates previous_block and deletes next_block to perform merge

                previous_block->size += allocation.size + next_block->size;
                previous_block->next = next_block->next;

                if ( next_block->next != nullptr )
                {
                    // Update the previous pointer of the next (noncontiguous) block to point to the newly coalesced block
                    next_block->next->previous = previous_block;
                }

                // Get rid of the 'third' coalesced block
                delete next_block;
            }
            else if ( previous_block->offset + previous_block->size == allocation.offset )
            {
                // This block is adjacent to the previous block, so just increase its size
                previous_block->size += allocation.size;
            }
            else if ( allocation.offset + allocation.size == next_block->offset )
            {
                // This block is adjacent to the next block, so update its offset and size
                next_block->offset -= allocation.size;
                next_block->size += allocation.size;
            }
            else
            {
                // This block is not adjacent to either neighboring blocks, so create a new block and link it into the list
                block* new_block     = new block( allocation.offset, allocation.size, previous_block, next_block );
                previous_block->next = new_block;
                next_block->previous = new_block;
            }
        }

        // Zero out allocation data so it isn't accidentally used in the future
        allocation.offset = 0;
        allocation.size   = 0;
        allocation.buffer = nullptr;
    }

    // Static helper function to find a suitable memory type (based on type_filter) that has all the required property flags (target_properties)
    uint find_best_memory_type( vk::PhysicalDevice device, bitmask type_filter, vk::MemoryPropertyFlags target_properties )
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
        check_error_condition( return 0, vulkan_log_errors, failed_to_find_memory_type == true, "Failed to find a suitable memory type" );
    }


    /* -------------------------------------------------------------------------- */
    /*                              Resource Database                             */
    /* -------------------------------------------------------------------------- */

    resource_database* resource_database::singleton = nullptr;

    resource_database::resource_database( const device& device_instance )
      : pass_member( device_instance ), //
        vertex_buffer_allocator(
            device_instance,                                                                //
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, //
            vk::MemoryPropertyFlagBits::eDeviceLocal ),
        index_buffer_allocator(
            device_instance,                                                               //
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, //
            vk::MemoryPropertyFlagBits::eDeviceLocal ),
        staging_buffer_allocator(
            device_instance,                                                                //
            vk::BufferUsageFlagBits::eTransferSrc, //
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent )
    {}
    resource_database::~resource_database() {}

    void resource_database::initialize()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan resource database initialization" );

        // Fixed number of elements each buffer has to work with
        let max_buffer_count = 500;

        vertex_buffer_allocator.initialize( sizeof( mesh::vertex ) * max_buffer_count );
        index_buffer_allocator.initialize( sizeof( mesh::index ) * max_buffer_count );
        staging_buffer_allocator.initialize( sizeof( mesh::vertex ) * max_buffer_count );

        resource_database::singleton = this;
    }

    // Copy host memory to CPU-accessible device memory owned by a staging buffer
    void resource_database::write_staging_buffer( const buffer_allocation& staging_buffer_allocation, vk::DeviceSize size, void* source )
    {
        let& vulkan_device = device_instance.get_vulkan_device();

        void* device_memory;
        let memory_map_flags = vk::MemoryMapFlags();

        vulkan_device.mapMemory(
            staging_buffer_allocator.get_memory(),  // memory
            staging_buffer_allocation.get_offset(), // offset
            staging_buffer_allocation.get_size(),   // size
            memory_map_flags,                       // flags
            &device_memory                          // ppData
        );
        memcpy( device_memory, source, size );
        vulkan_device.unmapMemory( staging_buffer_allocator.get_memory() );
    }

    // Copy CPU-accessible device memory to higher performance internal device memory
    // note: must record and submit a command buffer, so this method waits for that operation to complete before returning
    void resource_database::transfer_staging_buffer( const buffer_allocation& staging_buffer_allocation, const buffer_allocation& target_allocation )
    {
        let& vulkan_device = device_instance.get_vulkan_device();

        // Allocate a command buffer
        vk::CommandBufferAllocateInfo command_buffer_allocation_info(
            device_instance.command_pool.transfer(), // commandPool
            vk::CommandBufferLevel::ePrimary,        // level
            1                                        // commandBufferCount
        );
        vk::CommandBuffer transfer_command_buffer = vulkan_device.allocateCommandBuffers( command_buffer_allocation_info )[0];
        check_error_condition( return, vulkan_log_errors, not transfer_command_buffer, "Failed to create transfer command buffer" );

        // Prep transfer command buffer info structs
        vk::CommandBufferBeginInfo transfer_begin_info(    //
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit // flags
        );
        vk::BufferCopy buffer_copy_info(
            staging_buffer_allocation.get_offset(), // srcOffset
            target_allocation.get_offset(),         // dstOffset
            staging_buffer_allocation.get_size()    // size
        );
        vk::SubmitInfo transfer_submit_info(
            0,                        // waitSemaphoreCount
            nullptr,                  // pWaitSemaphores
            {},                       // pWaitDstStageMask
            1,                        // commandBufferCount
            &transfer_command_buffer, // pCommandBuffers
            0,                        // signalSemaphoreCount
            nullptr                   // pSignalSemaphores
        );
        let no_fence = vk::Fence();

        // Record the transfer command buffer and immediately submit
        transfer_command_buffer.begin( transfer_begin_info );
        transfer_command_buffer.copyBuffer( staging_buffer_allocation.get_buffer(), target_allocation.get_buffer(), 1, &buffer_copy_info );
        transfer_command_buffer.end();

        device_instance.queue.graphics().submit( 1, &transfer_submit_info, no_fence );

        // Wait for the transfer to complete before continuing
        device_instance.queue.graphics().waitIdle();

        // Free transfer command buffer
        vulkan_device.freeCommandBuffers( device_instance.command_pool.transfer(), 1, &transfer_command_buffer );
    }

    // Release resources used by a staging buffer
    void resource_database::free_staging_buffer( buffer_allocation& staging_buffer_allocation )
    {
        staging_buffer_allocator.free( staging_buffer_allocation );
    }

    // Allocate a vertex buffer in internal device memory and transfer mesh data using a staging buffer
    buffer_allocation resource_database::create_vertex_buffer( const list<mesh::vertex>& vertices )
    {
        let buffer_size = sizeof( mesh::vertex ) * vertices.size();

        buffer_allocation new_vertex_buffer = vertex_buffer_allocator.allocate( buffer_size );

        // TODO: aggregate transfer requests and execute all at once, rather than creating, writing, and destroying staging buffers individually
        buffer_allocation new_staging_buffer = staging_buffer_allocator.allocate( buffer_size );
        write_staging_buffer( new_staging_buffer, buffer_size, (void*) vertices.data() );
        transfer_staging_buffer( new_staging_buffer, new_vertex_buffer );
        free_staging_buffer( new_staging_buffer );

        return new_vertex_buffer;
    }

    // Allocate an index buffer in internal device memory and transfer mesh data using a staging buffer
    buffer_allocation resource_database::create_index_buffer( const list<mesh::index>& indices )
    {
        let buffer_size = sizeof( mesh::index ) * indices.size();

        buffer_allocation new_index_buffer = index_buffer_allocator.allocate( buffer_size );

        // TODO: aggregate transfer requests and execute all at once, rather than creating, writing, and destroying staging buffers individually
        buffer_allocation new_staging_buffer = staging_buffer_allocator.allocate( buffer_size );
        write_staging_buffer( new_staging_buffer, buffer_size, (void*) indices.data() );
        transfer_staging_buffer( new_staging_buffer, new_index_buffer );
        free_staging_buffer( new_staging_buffer );

        return new_index_buffer;
    }

    // Free resources used by a mesh vertex buffer
    void resource_database::free_vertex_buffer( buffer_allocation& allocation )
    {
        vertex_buffer_allocator.free( allocation );
    }

    // Free resources used by a mesh index buffer
    void resource_database::free_index_buffer( buffer_allocation& allocation )
    {
        index_buffer_allocator.free( allocation );
    }


    /* -------------------------------------------------------------------------- */
    /*                        Internal Resources Component                        */
    /* -------------------------------------------------------------------------- */

    internal_resources::internal_resources() {}
    internal_resources::~internal_resources() {}

    // Allocate Vulkan resources based on those of a single model component
    void internal_resources::update( const model& source )
    {
        let_mutable* db = resource_database::get_instance();

        if ( vertices.is_valid() )
        {
            db->free_vertex_buffer( vertices );
        }
        if ( indices.is_valid() )
        {
            db->free_index_buffer( indices );
        }

        vertices = db->create_vertex_buffer( source.get_mesh().get_vertices() );
        indices  = db->create_index_buffer( source.get_mesh().get_indices() );
    }
} // namespace rnjin::graphics::vulkan