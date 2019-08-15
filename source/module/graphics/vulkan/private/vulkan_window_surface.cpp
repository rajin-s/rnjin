/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "render_view.hpp"
#include "vulkan_renderer_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        /**************************
         * Creation / Destruction *
         **************************/
        renderer<vulkan>::internal::window_surface::window_surface( const internal& parent ) : child_class( parent )
        {
            vulkan_log_verbose.print_additional( "Created Vulkan window surface" );
        }
        renderer<vulkan>::internal::window_surface::~window_surface()
        {
            vulkan_log_verbose.print_additional( "Destroyed Vulkan window surface" );
        }

        /*************
         * Rendering *
         *************/
        struct test_vertex
        {
            float2 position;
            float3 color;

            static const vk::VertexInputBindingDescription get_binding_description()
            {
                return vk::VertexInputBindingDescription(
                    0,                           // binding
                    sizeof( test_vertex ),       // stride
                    vk::VertexInputRate::eVertex // inputRate
                );
            }

            static const vk::VertexInputAttributeDescription* get_attribute_descriptions()
            {
                static const vk::VertexInputAttributeDescription descriptions[] = {
                    vk::VertexInputAttributeDescription(
                        0,                                // location
                        0,                                // binding
                        vk::Format::eR32G32Sfloat,        // format
                        offsetof( test_vertex, position ) // offset
                        ),
                    vk::VertexInputAttributeDescription(
                        1,                             // location
                        0,                             // binding
                        vk::Format::eR32G32B32Sfloat,  // format
                        offsetof( test_vertex, color ) // offset
                        ),
                };

                return descriptions;
            }
        };

        uint test_find_memory_type( vk::PhysicalDevice device, uint type_filter, vk::MemoryPropertyFlags properties )
        {
            vk::PhysicalDeviceMemoryProperties memory_properties = device.getMemoryProperties();
            for ( uint i : range( memory_properties.memoryTypeCount ) )
            {
                bool pass_filter     = type_filter & ( 1 << i );
                bool same_properties = ( memory_properties.memoryTypes[i].propertyFlags & properties ) == properties;

                if ( pass_filter and same_properties )
                {
                    return i;
                }
            }

            const bool failed_to_find_value = true;
            check_error_condition( return 0, vulkan_log_errors, failed_to_find_value == true, "Failed to find a suitable memory type" );
        }

        static vk::DeviceMemory test_vertex_buffer_memory;
        static vk::Buffer test_vertex_buffer;

        void renderer<vulkan>::internal::window_surface::render()
        {
            static shader test_vertex_shader( "TestVertexShader", shader::type::vertex );
            static shader test_fragment_shader( "TestFragmentShader", shader::type::fragment );
            static vk::Pipeline test_pipeline;

            let static test_vertices = {
                test_vertex{ { 0.0, -0.5 }, { 1.0, 1.0, 0.0 } }, //
                test_vertex{ { 0.5, 0.5 }, { 0.0, 1.0, 1.0 } },  //
                test_vertex{ { -0.5, 0.5 }, { 0.0, 1.0, 1.0 } }, //
            };

            if ( not test_pipeline )
            {
                let task = vulkan_log_verbose.track_scope( "Test pipeline creation" );

                test_vertex_shader.set_glsl( // VERTEX
                    "#version 450\n"
                    "#extension GL_ARB_separate_shader_objects : enable\n"
                    "\n"
                    "layout(location = 0) in vec2 inPosition;\n"
                    "layout(location = 1) in vec3 inColor;\n"
                    "\n"
                    "layout(location = 0) out vec3 fragColor;\n"
                    "\n"
                    "void main() {\n"
                    "    gl_Position = vec4(inPosition, 0.0, 1.0);\n"
                    "    fragColor = inColor;\n"
                    "}\n" );
                test_fragment_shader.set_glsl( // FRAGMENT
                    "#version 450\n"
                    "#extension GL_ARB_separate_shader_objects : enable\n"
                    "layout(location = 0) in vec3 fragColor;\n"
                    "layout(location = 0) out vec4 outColor;\n"
                    "void main() {\n"
                    "    outColor = vec4(fragColor, 1.0);\n"
                    "}\n" //
                );

                test_vertex_shader.compile();
                test_fragment_shader.compile();

                let pipeline_id = create_pipeline( test_vertex_shader, test_fragment_shader );
                test_pipeline   = pipelines[pipeline_id].vulkan_pipeline;

                vk::BufferCreateInfo buffer_info(
                    {},                                           // flags
                    sizeof( test_vertex ) * test_vertices.size(), // size
                    vk::BufferUsageFlagBits::eVertexBuffer,       // usage
                    vk::SharingMode::eExclusive                   // sharingMode
                );

                let& device        = parent.device.vulkan_device;
                test_vertex_buffer = device.createBuffer( buffer_info );

                let memory_requirements = device.getBufferMemoryRequirements( test_vertex_buffer );
                vk::MemoryAllocateInfo allocate_info(
                    memory_requirements.size, // allocationSize
                    test_find_memory_type( parent.device.physical_device, memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent ) // memoryTypeIndex
                );
                test_vertex_buffer_memory = device.allocateMemory( allocate_info );
                device.bindBufferMemory( test_vertex_buffer, test_vertex_buffer_memory, 0 );

                void* data;
                device.mapMemory( test_vertex_buffer_memory, 0, buffer_info.size, {}, &data );
                memcpy( data, test_vertices.begin(), buffer_info.size );
                device.unmapMemory( test_vertex_buffer_memory );
            }

            check_error_condition( return, vulkan_log_errors, not test_pipeline, "Test pipeline is invalid" );

            tracked_subregion( vulkan_log_verbose, "Submit Vulkan command buffer" ) // acquire next image and submit appropriate command buffer
            {
                let static no_fence = vk::Fence();                          // Null handle to specify that no fence should be used for a call
                let static timeout  = std::numeric_limits<uint64_t>::max(); // max value to sepcify no timeout for function calls

                let& device = parent.device.vulkan_device;

                // Get semaphores for current frame in flight
                let& image_available = swapchain.synchronization.frames[swapchain.synchronization.current_frame].image_available;
                let& render_finished = swapchain.synchronization.frames[swapchain.synchronization.current_frame].render_finished;
                let& in_flight       = swapchain.synchronization.frames[swapchain.synchronization.current_frame].in_flight;

                // Wait for the current frame to be available (not in flight)
                device.waitForFences( 1, &in_flight, true, timeout );

                // Get the next image from the swapchain
                let next_image = device.acquireNextImageKHR( swapchain.vulkan_swapchain, timeout, image_available, no_fence );

                // Detect and handle resizing (don't render anything)
                if ( next_image.result == vk::Result::eErrorOutOfDateKHR )
                {
                    handle_out_of_date_swapchain();
                    return;
                }

                // Mark the current frame as in flight
                device.resetFences( 1, &in_flight );

                let next_image_index = next_image.value;
                let& command_buffer  = swapchain.elements[next_image_index].command_buffer;

                subregion // Record command buffer
                {
                    let& frame_buffer = swapchain.elements[next_image_index].frame_buffer;

                    const vk::CommandBufferBeginInfo begin_info(
                        {},     // flags
                        nullptr // pInheritanceInfo
                    );

                    const vk::ClearValue clear_value;

                    const vk::Rect2D render_area(
                        vk::Offset2D(),                                                // offset
                        vk::Extent2D( swapchain.image_size.x, swapchain.image_size.y ) // size
                    );

                    list<vk::Viewport> viewports = {
                        vk::Viewport( 0.0, 0.0, swapchain.image_size.x, swapchain.image_size.y, 0.0, 1.0 ) //
                    };

                    static const list<vk::Buffer> vertex_buffers{ test_vertex_buffer };
                    static const list<vk::DeviceSize> offsets{ 0 };

                    vk::RenderPassBeginInfo render_pass_info(
                        render_pass,  // renderPass
                        frame_buffer, // framebuffer
                        render_area,  // renderArea
                        1,            // clearValueCount
                        &clear_value  // pClearValues
                    );

                    command_buffer.begin( begin_info );
                    command_buffer.beginRenderPass( render_pass_info, vk::SubpassContents::eInline );
                    command_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, test_pipeline );

                    command_buffer.setViewport( 0, viewports );
                    command_buffer.setScissor( 0, render_area );

                    command_buffer.bindVertexBuffers( 0, vertex_buffers, offsets );
                    command_buffer.draw( test_vertices.size(), 1, 0, 0 );

                    command_buffer.endRenderPass();
                    command_buffer.end();
                }

                const vk::Semaphore submit_wait_semaphores[]   = { image_available };
                const vk::Semaphore submit_signal_semaphores[] = { render_finished };

                const vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

                vk::SubmitInfo submit_info(
                    1,                       // waitSemaphoreCount
                    submit_wait_semaphores,  // pWaitSemaphores
                    wait_stages,             // pWaitDstStageMask
                    1,                       // commandBufferCount
                    &command_buffer,         // pCommandBuffers
                    1,                       // signalSemaphoreCount
                    submit_signal_semaphores // pSignalSemaphores
                );
                parent.device.queue.graphics.submit( 1, &submit_info, in_flight );

                vk::SwapchainKHR swapchains[] = { swapchain.vulkan_swapchain };
                vk::PresentInfoKHR present_info(
                    1,                        // waitSemaphoreCount
                    submit_signal_semaphores, // pWaitSemaphores
                    1,                        // swapchainCount
                    swapchains,               // pSwapchains
                    &next_image_index,        // pImageIndices
                    nullptr                   // pResults
                );

                try
                {
                    parent.device.queue.present.presentKHR( present_info );
                }
                catch ( vk::OutOfDateKHRError out_of_date )
                {
                    handle_out_of_date_swapchain();
                }

                // Advance to next frame in flight
                swapchain.synchronization.current_frame = ( swapchain.synchronization.current_frame + 1 ) % swapchain.synchronization.max_frames_in_flight;
            }
        }

        /***************************************
         * Vulkan structure initialization     *
         * (called from internal::initialize() *
         ***************************************/
        void renderer<vulkan>::internal::window_surface::create_surface( window<GLFW>& target )
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan window surface initialization" );

            window_pointer = &target;
            window_size    = uint2( target.get_size().x, target.get_size().y ); // todo: update window to use uint2

            let& instance             = parent.get_vulkan_instance();
            let create_surface_result = glfwCreateWindowSurface( instance, target.get_api_window(), nullptr, (VkSurfaceKHR*) &vulkan_surface );

            check_error_condition( return, vulkan_log_errors, create_surface_result != VK_SUCCESS, "Failed to create Vulkan surface from GLFW window" );
        }

        void renderer<vulkan>::internal::window_surface::create_swapchain()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan swapchain creation" );

            check_error_condition( return, vulkan_log_errors, parent.device.queue.family_index.graphics < 0, "Invalid graphics queue index" );
            check_error_condition( return, vulkan_log_errors, parent.device.queue.family_index.present < 0, "Invalid present queue index" );
            check_error_condition( return, vulkan_log_errors, not vulkan_surface, "Vulkan surface is invalid" );

            let& device          = parent.device.vulkan_device;
            let& physical_device = parent.device.physical_device;

            let capabilities            = physical_device.getSurfaceCapabilitiesKHR( vulkan_surface );
            let supported_formats       = physical_device.getSurfaceFormatsKHR( vulkan_surface );
            let supported_present_modes = physical_device.getSurfacePresentModesKHR( vulkan_surface );

            vk::SurfaceFormatKHR best_format     = get_best_surface_format( supported_formats );
            vk::PresentModeKHR best_present_mode = get_best_present_mode( supported_present_modes );
            uint2 best_size                      = get_best_swap_extent( capabilities, window_size );

            swapchain.format       = best_format;
            swapchain.present_mode = best_present_mode;
            swapchain.image_size   = best_size;

            // Don't exceed the maximum image count
            swapchain.image_count = capabilities.minImageCount + 1;
            if ( capabilities.maxImageCount > 0 and swapchain.image_count > capabilities.maxImageCount )
            {
                swapchain.image_count = capabilities.maxImageCount;
            }

            uint queue_indices[] = { parent.device.queue.family_index.graphics, parent.device.queue.family_index.present };

            // Try to be concurrent by default
            vk::SharingMode sharing_mode  = vk::SharingMode::eConcurrent;
            uint queue_family_index_count = 2;
            uint* queue_family_indices    = queue_indices;

            // Can't be concurrent if the graphics and present queues are the same
            if ( parent.device.queue.family_index.graphics == parent.device.queue.family_index.present )
            {
                sharing_mode             = vk::SharingMode::eExclusive;
                queue_family_index_count = 0;
                queue_family_indices     = nullptr;
            }

            vk::SwapchainCreateInfoKHR create_info(
                {},                                                             // flags
                vulkan_surface,                                                 // surface
                swapchain.image_count,                                          // minImageCount
                swapchain.format.format,                                        // imageFormat
                swapchain.format.colorSpace,                                    // imageColorSpace
                vk::Extent2D( swapchain.image_size.x, swapchain.image_size.y ), // imageExtent
                1,                                                              // imageArrayLayers
                vk::ImageUsageFlagBits::eColorAttachment,                       // imageUsage
                sharing_mode,                                                   // imageSharingMode
                queue_family_index_count,                                       // queueFamilyIndexCount
                queue_family_indices,                                           // pQueueFamilyIndices
                capabilities.currentTransform,                                  // preTransform
                vk::CompositeAlphaFlagBitsKHR::eOpaque,                         // compositeAlpha
                swapchain.present_mode,                                         // presentMode
                true,                                                           // clipped
                nullptr                                                         // oldSwapchain
            );

            swapchain.vulkan_swapchain = device.createSwapchainKHR( create_info );
            check_error_condition( return, vulkan_log_errors, not swapchain.vulkan_swapchain, "Failed to create Vulkan swapchain" );

            let swapchain_images  = device.getSwapchainImagesKHR( swapchain.vulkan_swapchain );
            swapchain.image_count = swapchain_images.size();
            swapchain.elements.resize( swapchain.image_count );

            for ( uint i : range( swapchain.image_count ) )
            {
                let& image    = swapchain_images[i];
                auto& element = swapchain.elements[i];

                vk::ImageViewCreateInfo image_view_info(
                    {},                      // flags
                    image,                   // image
                    vk::ImageViewType::e2D,  // viewType
                    swapchain.format.format, // format
                    vk::ComponentMapping(),  // components

                    vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 ) // subresourceRange
                );

                element.image      = image;
                element.image_view = device.createImageView( image_view_info );
            }

            vulkan_log_verbose.print_additional( "Created \1 swapchain elements", swapchain.elements.size() );
        }

        void renderer<vulkan>::internal::window_surface::create_render_pass()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan render pass creation" );

            vk::AttachmentDescription color_attachment(
                {},                               // flags
                swapchain.format.format,          // format
                vk::SampleCountFlagBits::e1,      // samples
                vk::AttachmentLoadOp::eClear,     // loadOp
                vk::AttachmentStoreOp::eStore,    // storeOp
                vk::AttachmentLoadOp::eDontCare,  // stencilLoadOp
                vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
                vk::ImageLayout::eUndefined,      // initialLayout
                vk::ImageLayout::ePresentSrcKHR   // finalLayout
            );

            vk::AttachmentReference color_attachment_reference(
                0,                                       // attachment
                vk::ImageLayout::eColorAttachmentOptimal // layout
            );

            vk::SubpassDescription subpass(
                {},                               // flags,
                vk::PipelineBindPoint::eGraphics, // pipelineBindPoint,
                0,                                // inputAttachmentCount,
                nullptr,                          // pInputAttachments,
                1,                                // colorAttachmentCount,
                &color_attachment_reference,      // pColorAttachments,
                0,                                // pResolveAttachments,
                nullptr,                          // pDepthStencilAttachment,
                0,                                // preserveAttachmentCount,
                nullptr                           // pPreserveAttachments,
            );

            vk::SubpassDependency subpass_dependency(
                VK_SUBPASS_EXTERNAL, // srcSubpass
                0,                   // dstSubpass

                vk::PipelineStageFlagBits::eColorAttachmentOutput, // srcStageMask
                vk::PipelineStageFlagBits::eColorAttachmentOutput, // dstStageMask

                {},                                                                                   // srcAccessMask
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, // dstAccessMask

                {} // dependencyFlags
            );

            vk::RenderPassCreateInfo render_pass_info(
                {},                 // flags
                1,                  // attachmentCount
                &color_attachment,  // pAttachments
                1,                  // subpassCount
                &subpass,           // pSubpasses
                1,                  // dependencyCount
                &subpass_dependency // pDependencies
            );

            render_pass = parent.device.vulkan_device.createRenderPass( render_pass_info );
            check_error_condition( return, vulkan_log_errors, not render_pass, "Failed to create Vulkan render pass" );
        }

        void renderer<vulkan>::internal::window_surface::create_frame_buffers()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan frame buffer creation" );

            for ( uint i : range( swapchain.image_count ) )
            {
                auto& element = swapchain.elements[i];

                vk::ImageView frame_buffer_attachments[] = { element.image_view };
                vk::FramebufferCreateInfo framebuffer_info(
                    {},                       // flags
                    render_pass,              // renderPass
                    1,                        // attachmentCount
                    frame_buffer_attachments, // pAttachments
                    swapchain.image_size.x,   // width
                    swapchain.image_size.y,   // height
                    1                         // layers
                );

                element.frame_buffer = parent.device.vulkan_device.createFramebuffer( framebuffer_info );
                check_error_condition( return, vulkan_log_errors, not element.frame_buffer, "Failed to create Vulkan frame buffer" );
            }
        }

        void renderer<vulkan>::internal::window_surface::create_command_buffers()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan command buffer creation" );

            vk::CommandBufferAllocateInfo allocate_info(
                parent.device.command_pool,       // commandPool
                vk::CommandBufferLevel::ePrimary, // level
                swapchain.image_count             // commandBufferCount
            );

            let command_buffers = parent.device.vulkan_device.allocateCommandBuffers( allocate_info );
            check_error_condition( return, vulkan_log_errors, command_buffers.empty(), "Failed to create Vulkan command buffers" );

            for ( uint i : range( swapchain.image_count ) )
            {
                swapchain.elements[i].command_buffer = command_buffers[i];
            }
        }

        void renderer<vulkan>::internal::window_surface::initialize_synchronization()
        {
            let& device = parent.device.vulkan_device;

            vk::SemaphoreCreateInfo semaphore_info;
            vk::FenceCreateInfo fence_info( vk::FenceCreateFlagBits::eSignaled );

            swapchain.synchronization.frames.resize( swapchain.synchronization.max_frames_in_flight );
            for ( uint i : range( swapchain.synchronization.max_frames_in_flight ) )
            {
                swapchain.synchronization.frames[i].image_available = device.createSemaphore( semaphore_info );
                swapchain.synchronization.frames[i].render_finished = device.createSemaphore( semaphore_info );
                swapchain.synchronization.frames[i].in_flight       = device.createFence( fence_info );

                check_error_condition( pass, vulkan_log_errors, not swapchain.synchronization.frames[i].image_available, "Failed to create Vulkan semaphore (i=\1)", i );
                check_error_condition( pass, vulkan_log_errors, not swapchain.synchronization.frames[i].render_finished, "Failed to create Vulkan semaphore (i=\1)", i );
                check_error_condition( pass, vulkan_log_errors, not swapchain.synchronization.frames[i].in_flight, "Failed to create Vulkan fence (i=\1)", i );
            }

            swapchain.synchronization.current_frame = 0;
        }

        uint renderer<vulkan>::internal::window_surface::create_pipeline( const shader& vertex_shader, const shader& fragment_shader )
        {
            static const uint invalid_pipeline_id = ~0;

            let task = vulkan_log_verbose.track_scope( "Vulkan pipeline creation" );

            const uint next_index      = pipelines.size();
            pipeline_info& result      = pipelines.emplace_back();
            vk::PipelineLayout& layout = result.layout;
            vk::Pipeline& pipeline     = result.vulkan_pipeline;

            let& device = parent.device.vulkan_device;

            // Get shader binaries from shaders
            // note: has_spirv() should be checked before getting here
            let& vertex_shader_binary   = vertex_shader.get_spirv();
            let& fragment_shader_binary = fragment_shader.get_spirv();

            // Get shader bytecode for each stage
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

            // Create vulkan shader modules
            // note: these will be cleaned up automatically since they're made with a createUnique call
            let vertex_shader_module   = device.createShaderModuleUnique( vertex_shader_info );
            let fragment_shader_module = device.createShaderModuleUnique( fragment_shader_info );

            check_error_condition( return invalid_pipeline_id, vulkan_log_errors, not vertex_shader_module, "Failed to create vertex shader module from shader '\1'", vertex_shader.get_name() );
            check_error_condition( return invalid_pipeline_id, vulkan_log_errors, not fragment_shader_module, "Failed to create vertex shader module from shader '\1'", fragment_shader.get_name() );

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

            let binding_description    = test_vertex::get_binding_description();
            let attribute_descriptions = test_vertex::get_attribute_descriptions();

            vk::PipelineVertexInputStateCreateInfo vertex_input_info(
                {},                    // flags
                1,                     // vertexBindingDescriptionCount
                &binding_description,  // pVertexBindingDescriptions
                2,                     // vertexAttributeDescriptionCount
                attribute_descriptions // pVertexAttributeDescriptions
            );

            vk::PipelineInputAssemblyStateCreateInfo input_assembly(
                {},                                   // flags
                vk::PrimitiveTopology::eTriangleList, // topology
                false                                 // primitiveRestartEnable
            );

            let viewport_size = swapchain.image_size;
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

            // Dynamic state???

            vk::PipelineLayoutCreateInfo pipeline_layout_info(
                {},      // flags
                0,       // setLayoutCount
                nullptr, // pSetLayouts
                0,       // pushConstantRangeCount
                nullptr  // pPushConstantRanges
            );

            layout = device.createPipelineLayout( pipeline_layout_info );
            check_error_condition( return invalid_pipeline_id, vulkan_log_errors, not layout, "Failed to create Vulkan pipeline layout" );

            vulkan_log_verbose.print_additional( "Created pipeline layout" );

            static let pipeline_dynamic = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamic_state(
                {},                      // flags
                pipeline_dynamic.size(), // dynamicStateCount
                pipeline_dynamic.begin() // pDynamicStates
            );

            // Create pipeline
            vk::GraphicsPipelineCreateInfo graphics_pipeline_info(
                {},                 // flags
                2,                  // stageCount
                shader_stages,      // pStages
                &vertex_input_info, // pVertexInputState
                &input_assembly,    // pInputAssemblyState
                nullptr,            // pTessellationState
                &viewport_state,    // pViewportState
                &rasterizer,        // pRasterizationState
                &multisampling,     // pMultisampleState
                nullptr,            // pDepthStencilState
                &color_blending,    // pColorBlendState
                &dynamic_state,     // pDynamicState
                layout,             // layout
                render_pass         // renderPass
                                    // subpass            = 0
                                    // basePipelineHandle = null
                                    // basePipelineIndex  = 0
            );

            pipeline = device.createGraphicsPipelines( vk::PipelineCache(), { graphics_pipeline_info } )[0];
            check_error_condition( return invalid_pipeline_id, vulkan_log_errors, not pipeline, "Failed to create Vulkan pipeline" );

            vulkan_log_verbose.print_additional( "Created graphics pipeline (id: \1)", next_index );
            return next_index;
        }

        /***************************
         * Vulkan structure cleanup *
         ****************************/
        void renderer<vulkan>::internal::window_surface::destroy_surface()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan window surface cleanup" );

            let& instance = parent.get_vulkan_instance();
            instance.destroySurfaceKHR( vulkan_surface );
        }

        void renderer<vulkan>::internal::window_surface::destroy_swapchain()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan swapchain cleanup" );

            let& device = parent.device.vulkan_device;
            device.destroySwapchainKHR( swapchain.vulkan_swapchain );
        }

        void renderer<vulkan>::internal::window_surface::destroy_render_pass()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan render pass cleanup" );

            let& device = parent.device.vulkan_device;
            device.destroyRenderPass( render_pass );
        }

        void renderer<vulkan>::internal::window_surface::destroy_frame_buffers()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan swapchain element cleanup" );

            let& device = parent.device.vulkan_device;
            foreach ( element : swapchain.elements )
            {
                device.destroyFramebuffer( element.frame_buffer );
                device.destroyImageView( element.image_view );
            }
        }

        void renderer<vulkan>::internal::window_surface::destroy_synchronization()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan synchronization cleanup" );

            let& device = parent.device.vulkan_device;
            foreach ( frame : swapchain.synchronization.frames )
            {
                device.destroySemaphore( frame.image_available );
                device.destroySemaphore( frame.render_finished );
                device.destroyFence( frame.in_flight );
            }
        }

        void renderer<vulkan>::internal::window_surface::destroy_pipelines()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan pipeline cleanup" );

            let& device = parent.device.vulkan_device;
            foreach ( pipeline : pipelines )
            {
                device.destroyPipeline( pipeline.vulkan_pipeline );
                device.destroyPipelineLayout( pipeline.layout );
            }

            device.destroyBuffer( test_vertex_buffer );
            device.freeMemory( test_vertex_buffer_memory );
        }

        /********************************
         * Vulkan structure re-creation *
         * (for window resize, etc.)    *
         ********************************/
        void renderer<vulkan>::internal::window_surface::handle_out_of_date_swapchain()
        {
            let task = vulkan_log_verbose.track_scope( "Vulkan swapchain out of date handling" );

            int2 new_window_size;
            glfwGetFramebufferSize( window_pointer->get_api_window(), &new_window_size.x, &new_window_size.y );

            check_error_condition( return, vulkan_log_errors, new_window_size.x < 0, "Vulkan window surface width (\1) is invalid", new_window_size.x );
            check_error_condition( return, vulkan_log_errors, new_window_size.y < 0, "Vulkan window surface height (\1) is invalid", new_window_size.y );

            handle_resize( uint2( new_window_size.x, new_window_size.y ) );
        }

        void renderer<vulkan>::internal::window_surface::handle_resize( const uint2 new_size )
        {
            window_size = new_size;

            // Wait for current rendering operations to finish before destroying resources
            parent.device.vulkan_device.waitIdle();

            destroy_frame_buffers();
            destroy_render_pass();
            destroy_swapchain();

            create_swapchain();
            create_render_pass();
            create_frame_buffers();
        }

        /****************************
         * Surface Creation Helpers *
         ****************************/
        const vk::SurfaceFormatKHR get_best_surface_format( const list<vk::SurfaceFormatKHR>& available_formats )
        {
            // Find a common easy-to-use format
            foreach ( format : available_formats )
            {
                if ( format.format == vk::Format::eB8G8R8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
                {
                    return format;
                }
            }

            // Default to first format
            // note: could do more logic to rank formats
            return available_formats[0];
        }

        const vk::PresentModeKHR get_best_present_mode( const list<vk::PresentModeKHR>& available_present_modes )
        {
            // fifo is a mostly guaranteed default, might need to use immediate
            vk::PresentModeKHR fallback = vk::PresentModeKHR::eFifo;

            // See if any of the available present modes are triple buffered, or if immediate is needed
            foreach ( present_mode : available_present_modes )
            {
                if ( present_mode == vk::PresentModeKHR::eMailbox )
                {
                    return present_mode;
                }
                else if ( present_mode == vk::PresentModeKHR::eImmediate )
                {
                    fallback = present_mode;
                }
            }

            return fallback;
        }

        const uint2 get_best_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities, const uint2 window_size )
        {
            uint2 extent;

            if ( capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max() )
            {
                extent.x = clamp( window_size.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
                extent.y = clamp( window_size.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );
            }
            else
            {
                extent.x = capabilities.currentExtent.width;
                extent.y = capabilities.currentExtent.height;
            }

            return extent;
        }
    } // namespace graphics
} // namespace rnjin