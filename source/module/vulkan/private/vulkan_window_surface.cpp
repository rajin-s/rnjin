/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_renderer_internal.hpp"
#include "vulkan_window_surface.hpp"

namespace rnjin::graphics::vulkan
{
    // Create an uninitialized window surface that has a reference to its parent renderer
    window_surface::window_surface( const renderer_internal& parent ) : render_target( parent )
    {
        vulkan_log_verbose.print_additional( "Created Vulkan window surface" );
    }
    // Destroy an uninitialized window surface
    // note: clean_up should have already been called
    window_surface::~window_surface()
    {
        vulkan_log_verbose.print_additional( "Destroyed Vulkan window surface" );
    }

    /// Rendering

    // Perform draw calls based on a provided render_view
    // note: all pre-draw-call optimization should happen during
    //       render_view generation, as this just blindly iterates
    //       over all elements.
    void window_surface::render( const render_view& view )
    {
        let static no_fence = vk::Fence();
        let static timeout  = std::numeric_limits<uint64_t>::max();
        let& device         = parent.device.vulkan_device;

        // Acquire next swapchain element, respecting synchronization
        let frame_synchronization = swapchain.synchronization.get_current_frame_info();

        // Wait for the current frame to be finished (no longer in flight from a previous render call)
        device.waitForFences( 1, &frame_synchronization.in_flight, true, timeout );

        // Get the next image from the swapchain
        let acquired_image = device.acquireNextImageKHR( swapchain.vulkan_swapchain, timeout, frame_synchronization.image_available, no_fence );

        // Detect and handle resizing (skip this render call)
        if ( acquired_image.result == vk::Result::eErrorOutOfDateKHR )
        {
            handle_out_of_date_swapchain();
            return;
        }

        // Mark the current frame as in flight and get swapchain element data
        device.resetFences( 1, &frame_synchronization.in_flight );
        let next_image_index    = acquired_image.value;
        auto& swapchain_element = swapchain.elements[next_image_index];

        // Begin the current frame's command buffer
        tracked_subregion( vulkan_log_verbose, "Begin command buffer" )
        {
            let begin_info = begin_frame_info{
                render_pass,                    // render_pass
                swapchain_element.frame_buffer, // frame_buffer
                swapchain.image_size,           // image_size
                true,                           // clear
                vk::ClearValue()                // clear_value
            };
            begin_frame( begin_info, swapchain_element.command_buffer );
        }

        // Do draw calls
        tracked_subregion( vulkan_log_verbose, "Record command buffer" )
        {
            // TODO: Support instance batching
            foreach ( item : view.get_items() )
            {
                let& mesh_data     = parent.resources.get_mesh_data( item.mesh_resource.get_id() );
                let& material_data = parent.resources.get_material_data( item.material_resource.get_id() );

                check_error_condition( continue, vulkan_log_errors, mesh_data.is_valid == false, "Can't draw without resource_database mesh data (id=\1)", item.mesh_resource.get_id() );
                check_error_condition( continue, vulkan_log_errors, material_data.is_valid == false, "Can't draw without resource_database material data (id=\1)", item.mesh_resource.get_id() );

                let draw_info = draw_call_info{
                    mesh_data.vertex_buffer, // vertex_buffer
                    mesh_data.index_buffer,  // index_buffer
                    material_data.pipeline,  // pipeline
                    mesh_data.vertex_count,  // vertex_count
                    mesh_data.index_count,   // index_count
                };
                draw( draw_info, swapchain_element.command_buffer );
            }
        }

        // End the current frame's command buffer
        tracked_subregion( vulkan_log_verbose, "End command buffer" )
        {
            end_frame( swapchain_element.command_buffer );
        }

        // Submit the current frame's command buffer
        tracked_subregion( vulkan_log_verbose, "Submit Vulkan command buffer" )
        {
            const vk::Semaphore submit_wait_semaphores[]   = { frame_synchronization.image_available };
            const vk::Semaphore submit_signal_semaphores[] = { frame_synchronization.render_finished };
            const vk::PipelineStageFlags wait_stages[]     = { { vk::PipelineStageFlagBits::eColorAttachmentOutput } };

            vk::SubmitInfo submit_info(
                1,                                 // waitSemaphoreCount
                submit_wait_semaphores,            // pWaitSemaphores
                wait_stages,                       // pWaitDstStageMask
                1,                                 // commandBufferCount
                &swapchain_element.command_buffer, // pCommandBuffers
                1,                                 // signalSemaphoreCount
                submit_signal_semaphores           // pSignalSemaphores
            );
            parent.device.queue.graphics.submit( 1, &submit_info, frame_synchronization.in_flight );

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
            swapchain.synchronization.advance_frame();
        }
    }

    // Create a Vulkan surface object from the provided window
    // note: this is called before initialization, as having a surface
    //       is useful for the renderer to pick the best physical device
    void window_surface::create_surface( window<GLFW>& target )
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan window surface initialization" );

        window_pointer = &target;
        window_size    = uint2( target.get_size().x, target.get_size().y ); // todo: update window to use uint2

        let& instance             = parent.get_vulkan_instance();
        let create_surface_result = glfwCreateWindowSurface( instance, target.get_api_window(), nullptr, (VkSurfaceKHR*) &vulkan_surface );

        check_error_condition( return, vulkan_log_errors, create_surface_result != VK_SUCCESS, "Failed to create Vulkan surface from GLFW window" );
    }

    // note: called from renderer_internal::initialize()
    void window_surface::initialize()
    {
        create_swapchain();
        create_render_pass();
        create_frame_buffers();
        create_command_buffers();
        initialize_synchronization();
    }

    // note: called from renderer_internal::clean_up()
    void window_surface::clean_up()
    {
        // destroy_pipelines();
        destroy_synchronization();
        destroy_frame_buffers();
        destroy_render_pass();
        destroy_swapchain();
        destroy_surface();
    }

    void window_surface::create_swapchain()
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
        if ( capabilities.maxImageCount > 0 and swapchain.image_count > capabilities.maxImageCount ) { swapchain.image_count = capabilities.maxImageCount; }

        // note: queue indices have already been checked to ensure they're non-negative
        uint queue_indices[] = { (uint) parent.device.queue.family_index.graphics, (uint) parent.device.queue.family_index.present };

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

    void window_surface::create_render_pass()
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

    void window_surface::create_frame_buffers()
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

    void window_surface::create_command_buffers()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan command buffer creation" );

        vk::CommandBufferAllocateInfo allocate_info(
            parent.device.command_pool,       // commandPool
            vk::CommandBufferLevel::ePrimary, // level
            swapchain.image_count             // commandBufferCount
        );

        let command_buffers = parent.device.vulkan_device.allocateCommandBuffers( allocate_info );
        check_error_condition( return, vulkan_log_errors, command_buffers.empty(), "Failed to create Vulkan command buffers" );

        for ( uint i : range( swapchain.image_count ) ) { swapchain.elements[i].command_buffer = command_buffers[i]; }
    }

    void window_surface::initialize_synchronization()
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

    /***************************
     * Vulkan structure cleanup *
     ****************************/
    void window_surface::destroy_surface()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan window surface cleanup" );

        let& instance = parent.get_vulkan_instance();
        instance.destroySurfaceKHR( vulkan_surface );
    }

    void window_surface::destroy_swapchain()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan swapchain cleanup" );

        let& device = parent.device.vulkan_device;
        device.destroySwapchainKHR( swapchain.vulkan_swapchain );
    }

    void window_surface::destroy_render_pass()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan render pass cleanup" );

        let& device = parent.device.vulkan_device;
        device.destroyRenderPass( render_pass );
    }

    void window_surface::destroy_frame_buffers()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan swapchain element cleanup" );

        let& device = parent.device.vulkan_device;
        foreach ( element : swapchain.elements )
        {
            device.destroyFramebuffer( element.frame_buffer );
            device.destroyImageView( element.image_view );
        }
    }

    void window_surface::destroy_synchronization()
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

    // void window_surface::destroy_pipelines()
    // {
    //     let task = vulkan_log_verbose.track_scope( "Vulkan pipeline cleanup" );

    //     let& device = parent.device.vulkan_device;
    //     foreach ( pipeline : pipelines )
    //     {
    //         device.destroyPipeline( pipeline.vulkan_pipeline );
    //         device.destroyPipelineLayout( pipeline.layout );
    //     }

    //     device.destroyBuffer( test_vertex_buffer );
    //     device.freeMemory( test_vertex_buffer_memory );
    // }

    /********************************
     * Vulkan structure re-creation *
     * (for window resize, etc.)    *
     ********************************/
    void window_surface::handle_out_of_date_swapchain()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan swapchain out of date handling" );

        int2 new_window_size;
        glfwGetFramebufferSize( window_pointer->get_api_window(), &new_window_size.x, &new_window_size.y );

        check_error_condition( return, vulkan_log_errors, new_window_size.x < 0, "Vulkan window surface width (\1) is invalid", new_window_size.x );
        check_error_condition( return, vulkan_log_errors, new_window_size.y < 0, "Vulkan window surface height (\1) is invalid", new_window_size.y );

        handle_resize( uint2( new_window_size.x, new_window_size.y ) );
    }

    void window_surface::handle_resize( const uint2 new_size )
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
            if ( format.format == vk::Format::eB8G8R8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear ) { return format; }
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
            if ( present_mode == vk::PresentModeKHR::eMailbox ) { return present_mode; }
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
} // namespace rnjin::graphics::vulkan