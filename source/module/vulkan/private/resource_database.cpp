/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_renderer_internal.hpp"
#include "resource_database.hpp"

namespace rnjin::graphics::vulkan
{
    resource_database::resource_database( const renderer_internal& parent ) : child_class( parent ) {}
    resource_database::~resource_database() {}

    // Set up references used throughout resource_database lifetime
    void resource_database::initialize()
    {
        let& device = parent.device.vulkan_device;

        tracked_subregion( vulkan_log_verbose, "Vulkan pipeline cache creation" )
        {
            vk::PipelineCacheCreateInfo pipeline_cache_info(
                {},     // flags
                0,      // initialDataSize
                nullptr // pInitialData
            );
            pipeline_cache = device.createPipelineCache( pipeline_cache_info );
        }

        // // Handle global mesh load/unload events
        // this->handle_event( mesh::events.mesh_loaded(), &resource_database::on_mesh_loaded );
        // this->handle_event( mesh::events.mesh_destroyed(), &resource_database::on_mesh_destroyed );

        // // Handle global material load/unload events
        // this->handle_event( material::events.material_loaded(), &resource_database::on_material_loaded );
        // this->handle_event( material::events.material_destroyed(), &resource_database::on_material_destroyed );
    }

    // Clean up and release data used by the database and its entries
    void resource_database::clean_up()
    {
        let& device = parent.device.vulkan_device;

        tracked_subregion( vulkan_log_verbose, "Vulkan mesh resources cleanup" )
        {
            // Collect remaining mesh resource IDs
            let mesh_count = mesh_data.size();

            list<resource::id> mesh_resource_ids;
            mesh_resource_ids.resize( mesh_count );

            foreach ( entry : mesh_data )
            {
                mesh_resource_ids.emplace_back( entry.first );
            }

            // Release remaining mesh resources
            foreach ( id : mesh_resource_ids )
            {
                release_mesh_data( id );
            }
        }

        tracked_subregion( vulkan_log_verbose, "Vulkan material resources cleanup" )
        {
            // Collect remaining material resource IDs
            let material_count = material_data.size();

            list<resource::id> material_resource_ids;
            material_resource_ids.resize( material_count );

            foreach ( entry : mesh_data )
            {
                material_resource_ids.emplace_back( entry.first );
            }

            // Release remaining material resources
            foreach ( id : material_resource_ids )
            {
                release_material_data( id );
            }
        }

        tracked_subregion( vulkan_log_verbose, "Vulkan pipeline cache cleanup" )
        {
            device.destroyPipelineCache( pipeline_cache );
        }
    }

    // Store mesh data every time a mesh is loaded
    void resource_database::on_mesh_loaded( const mesh& mesh_resource )
    {
        vulkan_log_verbose.print( "resource_database: handle load mesh (\1, path='\2')", mesh_resource.get_id(), mesh_resource.get_path() );
        store_mesh_data( mesh_resource );
    }

    // Release mesh data when a mesh is destroyed
    void resource_database::on_mesh_destroyed( const mesh& mesh_resource )
    {
        vulkan_log_verbose.print( "resource_database: handle destroy mesh (\1, path='\2')", mesh_resource.get_id(), mesh_resource.get_path() );
        release_mesh_data( mesh_resource.get_id() );
    }
    void resource_database::on_material_loaded( const material& material_resource )
    {
        vulkan_log_verbose.print( "resource_database: handle load material (\1, path='\2')", material_resource.get_id(), material_resource.get_path() );
        store_material_data( material_resource );
    }

    // Release mesh data when a mesh is destroyed
    void resource_database::on_material_destroyed( const material& material_resource )
    {
        vulkan_log_verbose.print( "resource_database: handle destroy material (\1, path='\2')", material_resource.get_id(), material_resource.get_path() );
        release_material_data( material_resource.get_id() );
    }

    /// Mesh data management ///
    resource_database::mesh_entry::mesh_entry() : is_valid( false ) {}
    resource_database::mesh_entry::mesh_entry( vk::Buffer vertex_buffer, vk::DeviceMemory vertex_buffer_memory, uint vertex_count, vk::Buffer index_buffer, vk::DeviceMemory index_buffer_memory, uint index_count )
      : is_valid( true ), pass_member( vertex_buffer ), pass_member( index_buffer ), pass_member( vertex_buffer_memory ), pass_member( index_buffer_memory ), pass_member( vertex_count ), pass_member( index_count )
    {}

    using mesh_entry             = resource_database::mesh_entry;
    let static invalid_mesh_data = mesh_entry();

    // note: declared here, defined below
    static const uint find_best_memory_type( vk::PhysicalDevice device, bitmask type_filter, vk::MemoryPropertyFlags target_properties );

    // Create an arbitrary buffer and allocate device memory
    // note: no data is transferred into the buffer by this method
    resource_database::create_buffer_result resource_database::create_buffer( usize size, vk::BufferUsageFlags usage_flags, vk::MemoryPropertyFlags memory_property_flags )
    {
        let& device = parent.device.vulkan_device;

        let buffer_sharing_mode = vk::SharingMode::eExclusive;
        let buffer_memory_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        vk::Buffer new_buffer;
        vk::DeviceMemory new_buffer_memory;

        // Create the buffer (no memory allocated yet)
        vk::BufferCreateInfo buffer_info(
            {},                 // flags
            size,               // size
            usage_flags,        // usage
            buffer_sharing_mode // sharingMode
        );
        new_buffer = device.createBuffer( buffer_info );
        check_error_condition( return create_buffer_result(), vulkan_log_errors, not new_buffer, "Failed to create Vulkan buffer object (size=\1)", size );

        // Get memory information based on buffer requirements
        let buffer_memory_requirements = device.getBufferMemoryRequirements( new_buffer );
        let buffer_memory_type_index   = find_best_memory_type( parent.device.physical_device, buffer_memory_requirements.memoryTypeBits, memory_property_flags );

        // Allocate memory for the buffer
        vk::MemoryAllocateInfo buffer_allocation_info(
            buffer_memory_requirements.size, // allocationSize
            buffer_memory_type_index         // memoryTypeIndex
        );
        new_buffer_memory = device.allocateMemory( buffer_allocation_info );
        check_error_condition( return create_buffer_result(), vulkan_log_errors, not new_buffer_memory, "Failed to allocate device memory for Vulkan buffer object (size=\1)", size );

        device.bindBufferMemory( new_buffer, new_buffer_memory, 0 );

        // Copy data from mesh into device memory
        // void* buffer_data;
        // device.bindBufferMemory( new_buffer, new_buffer_memory, 0 );
        // device.mapMemory( new_buffer_memory, 0, buffer_info.size, vk::MemoryMapFlags(), &buffer_data );
        // memcpy( buffer_data, mesh_vertices.data(), buffer_info.size );
        // device.unmapMemory( new_vertex_buffer_memory );

        return create_buffer_result( new_buffer, new_buffer_memory );
    }

    // Copy the contents of one buffer into another
    // note: happens using a transfer command submitted into a command buffer
    void resource_database::copy_buffer( vk::Buffer source, vk::Buffer destination, usize size )
    {
        let& device = parent.device.vulkan_device;

        vk::CommandBufferAllocateInfo command_buffer_allocation_info(
            parent.device.transfer_command_pool, // commandPool
            vk::CommandBufferLevel::ePrimary,    // level
            1                                    // commandBufferCount
        );

        vk::CommandBuffer transfer_command_buffer = device.allocateCommandBuffers( command_buffer_allocation_info )[0];
        check_error_condition( return, vulkan_log_errors, not transfer_command_buffer, "Failed to create transfer command buffer" );

        // Record the transfer command buffer and immediately submit
        vk::CommandBufferBeginInfo transfer_begin_info(    //
            vk::CommandBufferUsageFlagBits::eOneTimeSubmit // flags
        );
        vk::BufferCopy buffer_copy_info(
            0,   // srcOffset
            0,   // dstOffset
            size // size
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

        transfer_command_buffer.begin( transfer_begin_info );
        transfer_command_buffer.copyBuffer( source, destination, 1, &buffer_copy_info );
        transfer_command_buffer.end();
        parent.device.queue.graphics.submit( 1, &transfer_submit_info, no_fence );

        // Wait for the transfer to complete before continuing
        parent.device.queue.graphics.waitIdle();
        device.freeCommandBuffers( parent.device.transfer_command_pool, 1, &transfer_command_buffer );
    }

    // Get the Vulkan resource data for a mesh resource with the given ID
    const mesh_entry& resource_database::get_mesh_data( const resource::id resource_id ) const
    {
        check_error_condition( return invalid_mesh_data, vulkan_log_errors, mesh_data.count( resource_id ) == 0, "Failed to find Vulkan mesh resource (\1)", resource_id );
        return mesh_data.at( resource_id );
    }

    // Create Vulkan structures used by a mesh resource (buffers / device memory allocation) and transfer data
    const mesh_entry& resource_database::store_mesh_data( const mesh& mesh_resource )
    {
        let mesh_id = mesh_resource.get_id();

        let has_data_for_mesh = mesh_data.count( mesh_id ) > 0;
        check_error_condition( return mesh_data.at( mesh_id ), vulkan_log_errors, has_data_for_mesh == true, "Vulkan data for mesh #\1 already exists", mesh_id );

        let& device = parent.device.vulkan_device;

        vk::Buffer new_vertex_buffer;
        vk::DeviceMemory new_vertex_buffer_memory;

        vk::Buffer new_index_buffer;
        vk::DeviceMemory new_index_buffer_memory;

        let& mesh_vertices = mesh_resource.get_vertices();
        let& mesh_indices  = mesh_resource.get_indices();

        tracked_subregion( vulkan_log_verbose, "Vulkan vertex buffer creation" )
        {
            // Get data from mesh
            let buffer_size = sizeof( mesh::vertex ) * mesh_vertices.size();

            // Create the staging buffer
            let staging_buffer_usage_flags           = vk::BufferUsageFlagBits::eTransferSrc;
            let staging_buffer_memory_property_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            let staging_buffer_result                = create_buffer( buffer_size, staging_buffer_usage_flags, staging_buffer_memory_property_flags );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not staging_buffer_result.valid, "Failed to create vertex staging buffer for mesh resource (\1)", mesh_resource.get_id() );

            // Create the vertex buffer
            let buffer_usage_flags    = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
            let memory_property_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
            let create_result         = create_buffer( buffer_size, buffer_usage_flags, memory_property_flags );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not create_result.valid, "Failed to create vertex buffer for mesh resource (\1)", mesh_resource.get_id() );

            new_vertex_buffer        = create_result.buffer;
            new_vertex_buffer_memory = create_result.buffer_memory;

            // Copy data from mesh into staging buffer
            void* buffer_data;
            device.mapMemory( staging_buffer_result.buffer_memory, 0, buffer_size, vk::MemoryMapFlags(), &buffer_data );
            memcpy( buffer_data, mesh_vertices.data(), buffer_size );
            device.unmapMemory( staging_buffer_result.buffer_memory );

            // Perform a device-level transfer from the staging buffer into the final vertex buffer
            copy_buffer( staging_buffer_result.buffer, new_vertex_buffer, buffer_size );

            // Release staging buffer resources
            device.destroyBuffer( staging_buffer_result.buffer );
            device.freeMemory( staging_buffer_result.buffer_memory );
        }

        tracked_subregion( vulkan_log_verbose, "Vulkan index buffer creation" )
        {
            // Get data from mesh
            let buffer_size = sizeof( mesh::index ) * mesh_indices.size();

            // Create the staging buffer
            let staging_buffer_usage_flags           = vk::BufferUsageFlagBits::eTransferSrc;
            let staging_buffer_memory_property_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            let staging_buffer_result                = create_buffer( buffer_size, staging_buffer_usage_flags, staging_buffer_memory_property_flags );
            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not staging_buffer_result.valid, "Failed to create vertex staging buffer for mesh resource (\1)", mesh_resource.get_id() );

            let buffer_usage_flags    = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
            let memory_property_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
            let create_result         = create_buffer( buffer_size, buffer_usage_flags, memory_property_flags );

            check_error_condition( return invalid_mesh_data, vulkan_log_errors, not create_result.valid, "Failed to create index buffer for mesh resource (\1)", mesh_resource.get_id() );

            new_index_buffer        = create_result.buffer;
            new_index_buffer_memory = create_result.buffer_memory;

            // Copy data from mesh into staging buffer
            void* buffer_data;
            device.mapMemory( staging_buffer_result.buffer_memory, 0, buffer_size, vk::MemoryMapFlags(), &buffer_data );
            memcpy( buffer_data, mesh_indices.data(), buffer_size );
            device.unmapMemory( staging_buffer_result.buffer_memory );

            // Perform a device-level transfer from the staging buffer into the final index buffer
            copy_buffer( staging_buffer_result.buffer, new_index_buffer, buffer_size );

            // Release staging buffer resources
            device.destroyBuffer( staging_buffer_result.buffer );
            device.freeMemory( staging_buffer_result.buffer_memory );
        }

        // Create and insert the new entry into the database
        let new_entry = mesh_entry{
            new_vertex_buffer,           // vertex_buffer
            new_vertex_buffer_memory,    // vertex_buffer_memory
            (uint) mesh_vertices.size(), // vertex_count
            new_index_buffer,            // index_buffer
            new_index_buffer_memory,     // index_buffer_memory
            (uint) mesh_indices.size()   // index_count
        };
        mesh_data.insert_or_assign( mesh_id, new_entry );
        return mesh_data.at( mesh_id );
    }

    // Free Vulkan resources used by a mesh resource (buffers / device memory)
    void resource_database::release_mesh_data( const resource::id resource_id )
    {
        check_error_condition( return, vulkan_log_errors, mesh_data.count( resource_id ) == 0, "Failed to find Vulkan mesh resource (\1)", resource_id );

        // Wait for all pending operations to complete before cleaning up resources
        // (so no structures are destroyed while in use by command buffers)
        parent.wait_for_device_idle();

        let entry = mesh_data.at( resource_id );
        mesh_data.erase( resource_id );

        let& device = parent.device.vulkan_device;

        // Free vertex buffer resources
        device.destroyBuffer( entry.vertex_buffer );
        device.freeMemory( entry.vertex_buffer_memory );

        // Free index buffer resources
        device.destroyBuffer( entry.index_buffer );
        device.freeMemory( entry.index_buffer_memory );
    }

    /// Material/Shader data management ///
    resource_database::material_entry::material_entry() : is_valid( false ) {}
    resource_database::material_entry::material_entry( vk::PipelineLayout pipeline_layout, vk::Pipeline pipeline ) : is_valid( true ), pass_member( pipeline_layout ), pass_member( pipeline ) {}

    using material_entry             = resource_database::material_entry;
    let static invalid_material_data = material_entry();

    // Get binding information for a particular vertex type
    // note: all supported types need to be template specializations
    struct vertex_description
    {
        vk::VertexInputBindingDescription input;

        uint input_attribute_count;
        vk::VertexInputAttributeDescription* input_attributes;
    };

    template <typename T>
    static vertex_description get_vertex_binding_description()
    {
        static let invalid_description = vertex_description{ vk::VertexInputBindingDescription(), 0, nullptr };

        let invalid_vertex_type = true;
        check_error_condition( return invalid_description, vulkan_log_errors, invalid_vertex_type == true, "Failed to get binding description for invalid vertex type" );
    }
    template <>
    static vertex_description get_vertex_binding_description<mesh::vertex>()
    {
        static const vk::VertexInputBindingDescription input(
            0,                             // binding
            mesh::vertex_info.vertex_size, // stride
            vk::VertexInputRate::eVertex   // inputRate
        );

        // clang-format off
        static list<vk::VertexInputAttributeDescription> attributes{
            vk::VertexInputAttributeDescription(  // position
                0,                                // location
                0,                                // binding
                vk::Format::eR32G32B32Sfloat,     // format
                mesh::vertex_info.position_offset // offset
            ),
            vk::VertexInputAttributeDescription(  // normal
                1,                                // location
                0,                                // binding
                vk::Format::eR32G32B32Sfloat,     // format
                mesh::vertex_info.normal_offset   // offset
            ), 
            vk::VertexInputAttributeDescription(  // color
                2,                                // location
                0,                                // binding
                vk::Format::eR32G32B32A32Sfloat,  // format
                mesh::vertex_info.color_offset    // offset
            ), 
            vk::VertexInputAttributeDescription(  // uv
                3,                                // location
                0,                                // binding
                vk::Format::eR32G32Sfloat,        // format
                mesh::vertex_info.uv_offset       // offset
            )
        };
        // clang-format on

        return vertex_description{
            input,                    // input
            (uint) attributes.size(), // input_attribute_count
            attributes.data()         // input_attributes
        };
    }

    // Get the Vulkan resource data for a material resource with the given ID
    const material_entry& resource_database::get_material_data( const resource::id resource_id ) const
    {
        check_error_condition( return invalid_material_data, vulkan_log_errors, material_data.count( resource_id ) == 0, "Failed to find Vulkan material resource (\1)", resource_id );
        return material_data.at( resource_id );
    }

    // Create Vulkan structures used by a material resource (pipeline layout, pipeline)
    const material_entry& resource_database::store_material_data( const material& material_resource )
    {
        let material_id = material_resource.get_id();

        let has_data_for_material = material_data.count( material_id ) > 0;
        check_error_condition( return material_data.at( material_id ), vulkan_log_errors, has_data_for_material == true, "Vulkan data for material #\1 already exists", material_id );

        let& device = parent.device.vulkan_device;

        vk::PipelineLayout new_pipeline_layout;
        vk::Pipeline new_pipeline;


        tracked_subregion( vulkan_log_verbose, "Vulkan pipeline creation" )
        {
            // Pipeline layout info
            // TODO: handle descriptor sets and push constants
            vk::PipelineLayoutCreateInfo pipeline_layout_info(
                {},      // flags
                0,       // setLayoutCount
                nullptr, // pSetLayouts
                0,       // pushConstantRangeCount
                nullptr  // pPushConstantRanges
            );

            new_pipeline_layout = device.createPipelineLayout( pipeline_layout_info );
            check_error_condition( return invalid_material_data, vulkan_log_errors, not new_pipeline_layout, "Failed to create Vulkan pipeline layout" );

            // Get shader bytecode for each stage
            let& vertex_shader_binary   = material_resource.get_vertex_shader().get_spirv();
            let& fragment_shader_binary = material_resource.get_fragment_shader().get_spirv();

            // Build shader modules
            vk::ShaderModuleCreateInfo vertex_shader_info(
                {},                                                         // flags
                vertex_shader_binary.size() * sizeof( shader::spirv_char ), // codeSize
                vertex_shader_binary.data()                                 // pCode
            );
            vk::ShaderModuleCreateInfo fragment_shader_info(
                {},                                                           // flags
                fragment_shader_binary.size() * sizeof( shader::spirv_char ), // codeSize
                fragment_shader_binary.data()                                 // pCode
            );

            // note: these will be cleaned up automatically since they're made with a createUnique call
            let vertex_shader_module   = device.createShaderModuleUnique( vertex_shader_info );
            let fragment_shader_module = device.createShaderModuleUnique( fragment_shader_info );

            check_error_condition( return invalid_material_data, vulkan_log_errors, not vertex_shader_module, "Failed to create vertex shader module from shader" );
            check_error_condition( return invalid_material_data, vulkan_log_errors, not fragment_shader_module, "Failed to create vertex shader module from shader" );

            // Shader stage info
            vk::PipelineShaderStageCreateInfo vertex_shader_stage_info(
                {},                               // flags
                vk::ShaderStageFlagBits::eVertex, // stage
                *vertex_shader_module,            // module
                "main",                           // pName
                nullptr                           // pSpecializationInfo
            );
            vk::PipelineShaderStageCreateInfo fragment_shader_stage_info(
                {},                                 // flags
                vk::ShaderStageFlagBits::eFragment, // stage
                *fragment_shader_module,            // module
                "main",                             // pName
                nullptr                             // pSpecializationInfo
            );
            vk::PipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_stage_info, fragment_shader_stage_info };

            // Vertex input info (currently standardized for only one vertex type mesh::vertex)
            let vertex_description = get_vertex_binding_description<mesh::vertex>();

            vk::PipelineVertexInputStateCreateInfo vertex_input_info(
                {},                                       // flags
                1,                                        // vertexBindingDescriptionCount
                &vertex_description.input,                // pVertexBindingDescriptions
                vertex_description.input_attribute_count, // vertexAttributeDescriptionCount
                vertex_description.input_attributes       // pVertexAttributeDescriptions
            );
            vk::PipelineInputAssemblyStateCreateInfo input_assembly(
                {},                                   // flags
                vk::PrimitiveTopology::eTriangleList, // topology
                false                                 // primitiveRestartEnable
            );

            // Viewport state (with dummy info since this will be overwritten by dynamic state)
            let viewport_size = uint2();
            vk::Viewport viewport(
                0.0,             // x
                0.0,             // y
                viewport_size.x, // width
                viewport_size.y, // height
                0.0,             // minDepth
                1.0              // maxDepth
            );
            vk::Rect2D scissor(
                vk::Offset2D( 0, 0 ),                            // offset
                vk::Extent2D( viewport_size.x, viewport_size.y ) // extent
            );
            vk::PipelineViewportStateCreateInfo viewport_state(
                {},        // flags
                1,         // viewportCount
                &viewport, // pViewports
                1,         // scissorCount
                &scissor   // pScissors
            );

            // Rasterization state
            vk::PipelineRasterizationStateCreateInfo rasterizer(
                {},                          // flags
                false,                       // depthClampEnable
                false,                       // rasterizerDiscardEnable
                vk::PolygonMode::eFill,      // polygonMode
                vk::CullModeFlagBits::eBack, // cullMode
                vk::FrontFace::eClockwise,   // frontFace
                false,                       // depthBiasEnable
                0.0,                         // depthBiasConstantFactor
                0.0,                         // depthBiasClamp
                0.0,                         // depthBiasSlopeFactor
                1.0                          // lineWidth
            );

            vk::PipelineMultisampleStateCreateInfo multisampling(
                {},                          // flags
                vk::SampleCountFlagBits::e1, // rasterizationSamples
                false,                       // sampleShadingEnable
                1.0,                         // minSampleShading
                nullptr,                     // pSampleMask
                false,                       // alphaToCoverageEnable
                false                        // alphaToOneEnable
            );

            // Color output state
            // TODO: pull this from material info
            let color_write_all = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            vk::PipelineColorBlendAttachmentState color_blend_attachment(
                false,                  // blendEnable
                vk::BlendFactor::eOne,  // srcColorBlendFactor
                vk::BlendFactor::eZero, // dstColorBlendFactor
                vk::BlendOp::eAdd,      // colorBlendOp
                vk::BlendFactor::eOne,  // srcAlphaBlendFactor
                vk::BlendFactor::eZero, // dstAlphaBlendFactor
                vk::BlendOp::eAdd,      // alphaBlendOp
                color_write_all         // colorWriteMask
            );

            vk::PipelineColorBlendStateCreateInfo color_blending(
                {},                      // flags
                false,                   // logicOpEnable
                vk::LogicOp::eCopy,      // logicOp
                1,                       // attachmentCount
                &color_blend_attachment, // pAttachments
                { 0.0, 0.0, 0.0, 0.0 }   // blendConstants
            );

            static let pipeline_dynamic_state = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamic_state(
                {},                            // flags
                pipeline_dynamic_state.size(), // dynamicStateCount
                pipeline_dynamic_state.begin() // pDynamicStates
            );

            // Create pipeline
            vk::GraphicsPipelineCreateInfo graphics_pipeline_info(
                {},                      // flags
                2,                       // stageCount
                shader_stages,           // pStages
                &vertex_input_info,      // pVertexInputState
                &input_assembly,         // pInputAssemblyState
                nullptr,                 // pTessellationState
                &viewport_state,         // pViewportState
                &rasterizer,             // pRasterizationState
                &multisampling,          // pMultisampleState
                nullptr,                 // pDepthStencilState
                &color_blending,         // pColorBlendState
                &dynamic_state,          // pDynamicState
                new_pipeline_layout,     // layout
                parent.get_render_pass() // renderPass
                                         // subpass            = 0
                                         // basePipelineHandle = null
                                         // basePipelineIndex  = 0
            );

            new_pipeline = device.createGraphicsPipelines( pipeline_cache, { graphics_pipeline_info } )[0];
            check_error_condition( return invalid_material_data, vulkan_log_errors, not new_pipeline, "Failed to create Vulkan pipeline" );
        }

        // Create and insert the new entry into the database
        let new_entry = material_entry{ new_pipeline_layout, new_pipeline };
        material_data.insert_or_assign( material_id, new_entry );
        return material_data.at( material_id );
    }

    // Free Vulkan resources used by a material resource (pipeline layout, pipeline)
    void resource_database::release_material_data( const resource::id resource_id )
    {
        check_error_condition( return, vulkan_log_errors, material_data.count( resource_id ) == 0, "Failed to find Vulkan material resource (\1)", resource_id );

        // Wait for all pending operations to complete before cleaning up resources
        // (so no structures are destroyed while in use by command buffers)
        parent.wait_for_device_idle();

        let entry = material_data.at( resource_id );
        material_data.erase( resource_id );

        let& device = parent.device.vulkan_device;

        // Destroy pipeline
        device.destroyPipeline( entry.pipeline );
        // Destroy pipeline layout
        device.destroyPipelineLayout( entry.pipeline_layout );
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