/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_window_surface.hpp"

namespace rnjin::graphics::vulkan
{
    window_surface::window_surface( const device& device_instance ) : pass_member( device_instance )
    {
        vulkan_log_verbose.print( "Created Vulkan window surface" );
    }
    window_surface::~window_surface()
    {
        vulkan_log_verbose.print( "Destroying Vulkan window surface" );
        clean_up();
    }

    // Create a Vulkan surface object from the provided window
    // note: this is called before initialization, as having a surface
    //       is needed by device_instance to check for presentation support
    void window_surface::create_surface( window<GLFW>& target )
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan window surface initialization" );

        window_pointer = &target;
        window_size    = uint2( target.get_size().x, target.get_size().y ); // todo: update window to use uint2

        let& instance             = device_instance.get_vulkan_instance();
        let create_surface_result = glfwCreateWindowSurface( instance, target.get_api_window(), nullptr, (VkSurfaceKHR*) &vulkan_surface );

        check_error_condition( return, vulkan_log_errors, create_surface_result != VK_SUCCESS, "Failed to create Vulkan surface from GLFW window" );
    }

    void window_surface::initialize()
    {
        create_swapchain();
        create_render_pass();
        create_frame_buffers();
        create_command_buffers();
        initialize_synchronization();
    }

    void window_surface::clean_up()
    {
        destroy_synchronization();
        destroy_frame_buffers();
        destroy_render_pass();
        destroy_swapchain();
        destroy_surface();
    }

/* -------------------------------------------------------------------------- */
/*                               Initialization                               */
/* -------------------------------------------------------------------------- */
#pragma region initialization

    // Helpers for swapchain creation
    const vk::SurfaceFormatKHR get_best_surface_format( const list<vk::SurfaceFormatKHR>& available_formats );
    const vk::PresentModeKHR get_best_present_mode( const list<vk::PresentModeKHR>& available_present_modes );
    const uint2 get_best_swap_extent( const vk::SurfaceCapabilitiesKHR capabilities, const uint2 window_size );

    void window_surface::create_swapchain()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan swapchain creation" );

        check_error_condition( return, vulkan_log_errors, device_instance.queue.family_index.graphics() < 0, "Invalid graphics queue index" );
        check_error_condition( return, vulkan_log_errors, device_instance.queue.family_index.present() < 0, "Invalid present queue index" );
        check_error_condition( return, vulkan_log_errors, not vulkan_surface, "Vulkan surface is invalid" );

        let& vulkan_device   = device_instance.get_vulkan_device();
        let& physical_device = device_instance.get_physical_device();

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

        // note: queue indices have already been checked to ensure they're non-negative
        let graphics_index   = (uint) device_instance.queue.family_index.graphics();
        let present_index    = (uint) device_instance.queue.family_index.present();
        uint queue_indices[] = { graphics_index, present_index };

        // Try to be concurrent by default
        vk::SharingMode sharing_mode  = vk::SharingMode::eConcurrent;
        uint queue_family_index_count = 2;
        uint* queue_family_indices    = queue_indices;

        // Can't be concurrent if the graphics and present queues are the same
        if ( graphics_index == present_index )
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

        swapchain.vulkan_swapchain = vulkan_device.createSwapchainKHR( create_info );
        check_error_condition( return, vulkan_log_errors, not swapchain.vulkan_swapchain, "Failed to create Vulkan swapchain" );

        let swapchain_images  = vulkan_device.getSwapchainImagesKHR( swapchain.vulkan_swapchain );
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
            element.image_view = vulkan_device.createImageView( image_view_info );
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

        render_pass = device_instance.get_vulkan_device().createRenderPass( render_pass_info );
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

            element.frame_buffer = device_instance.get_vulkan_device().createFramebuffer( framebuffer_info );
            check_error_condition( return, vulkan_log_errors, not element.frame_buffer, "Failed to create Vulkan frame buffer" );
        }
    }

    void window_surface::create_command_buffers()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan command buffer creation" );

        vk::CommandBufferAllocateInfo allocate_info(
            device_instance.command_pool.main(), // commandPool
            vk::CommandBufferLevel::ePrimary,    // level
            swapchain.image_count                // commandBufferCount
        );

        let command_buffers = device_instance.get_vulkan_device().allocateCommandBuffers( allocate_info );
        check_error_condition( return, vulkan_log_errors, command_buffers.empty(), "Failed to create Vulkan command buffers" );

        for ( uint i : range( swapchain.image_count ) )
        {
            swapchain.elements[i].command_buffer = command_buffers[i];
        }
    }

    void window_surface::initialize_synchronization()
    {
        let& vulkan_device = device_instance.get_vulkan_device();

        vk::SemaphoreCreateInfo semaphore_info;
        vk::FenceCreateInfo fence_info( vk::FenceCreateFlagBits::eSignaled );

        swapchain.synchronization.frames.resize( swapchain.synchronization.max_frames_in_flight );
        for ( uint i : range( swapchain.synchronization.max_frames_in_flight ) )
        {
            swapchain.synchronization.frames[i].image_available = vulkan_device.createSemaphore( semaphore_info );
            swapchain.synchronization.frames[i].render_finished = vulkan_device.createSemaphore( semaphore_info );
            swapchain.synchronization.frames[i].in_flight       = vulkan_device.createFence( fence_info );

            check_error_condition( pass, vulkan_log_errors, not swapchain.synchronization.frames[i].image_available, "Failed to create Vulkan semaphore (i=\1)", i );
            check_error_condition( pass, vulkan_log_errors, not swapchain.synchronization.frames[i].render_finished, "Failed to create Vulkan semaphore (i=\1)", i );
            check_error_condition( pass, vulkan_log_errors, not swapchain.synchronization.frames[i].in_flight, "Failed to create Vulkan fence (i=\1)", i );
        }

        swapchain.synchronization.current_frame = 0;
    }
#pragma endregion initialization

/* -------------------------------------------------------------------------- */
/*                          Drawing / Synchronization                         */
/* -------------------------------------------------------------------------- */
#pragma region drawing

    void window_surface::begin_frame()
    {
        let static no_fence = vk::Fence();
        let static timeout  = std::numeric_limits<uint64>::max();

        let& vulkan_device = device_instance.get_vulkan_device();

        let& frame_synchronization = swapchain.synchronization.get_current_frame_info();
        {
            // Acquire next swapchain element, respecting synchronization
            // Wait for the current frame to be finished (no longer in flight from a previous render call)
            vulkan_device.waitForFences( 1, &frame_synchronization.in_flight, true, timeout );

            let acquired_image = vulkan_device.acquireNextImageKHR( swapchain.vulkan_swapchain, timeout, frame_synchronization.image_available, no_fence );
            if ( acquired_image.result == vk::Result::eErrorOutOfDateKHR )
            {
                handle_out_of_date_swapchain();
                begin_frame();
            }

            vulkan_device.resetFences( 1, &frame_synchronization.in_flight );

            swapchain.current_element_index = acquired_image.value;
        }

        let_mutable& current_swapchain_element = swapchain.elements[swapchain.current_element_index];
        {
            // Start the command buffer and render pass
            let viewport            = vk::Viewport( 0.0, 0.0, swapchain.image_size.x, swapchain.image_size.y );
            let_mutable clear_value = vk::ClearValue();
            let subpass_contents    = vk::SubpassContents::eInline;

            let begin_info = vk::CommandBufferBeginInfo(
                {},     // flags
                nullptr // pInheritanceInfo
            );
            let full_screen_area = vk::Rect2D(
                vk::Offset2D( 0, 0 ),                                          // origin
                vk::Extent2D( swapchain.image_size.x, swapchain.image_size.y ) // size
            );
            let render_pass_info = vk::RenderPassBeginInfo(
                render_pass,                            // renderPass
                current_swapchain_element.frame_buffer, // framebuffer
                full_screen_area,                       // renderArea
                1,                                      // clearValueCount
                &clear_value                            // pClearValues
            );

            current_swapchain_element.command_buffer.begin( begin_info );
            current_swapchain_element.command_buffer.beginRenderPass( render_pass_info, subpass_contents );
            current_swapchain_element.command_buffer.setViewport( 0, 1, &viewport );
            current_swapchain_element.command_buffer.setScissor( 0, 1, &full_screen_area );
        }
    }

    void window_surface::end_frame()
    {
        let_mutable& current_swapchain_element = swapchain.elements[swapchain.current_element_index];
        {
            // End the render pass and command buffer
            current_swapchain_element.command_buffer.endRenderPass();
            current_swapchain_element.command_buffer.end();
        }

        let& frame_synchronization = swapchain.synchronization.get_current_frame_info();
        {
            // Submit command buffer to graphics queue
            const vk::Semaphore submit_wait_semaphores[]   = { frame_synchronization.image_available };
            const vk::Semaphore submit_signal_semaphores[] = { frame_synchronization.render_finished };
            const vk::PipelineStageFlags wait_stages[]     = { { vk::PipelineStageFlagBits::eColorAttachmentOutput } };
            vk::SubmitInfo submit_info(
                1,                                         // waitSemaphoreCount
                submit_wait_semaphores,                    // pWaitSemaphores
                wait_stages,                               // pWaitDstStageMask
                1,                                         // commandBufferCount
                &current_swapchain_element.command_buffer, // pCommandBuffers
                1,                                         // signalSemaphoreCount
                submit_signal_semaphores                   // pSignalSemaphores
            );
            device_instance.queue.graphics().submit( 1, &submit_info, frame_synchronization.in_flight );

            // Do presentation
            vk::SwapchainKHR swapchains[] = { swapchain.vulkan_swapchain };
            vk::PresentInfoKHR present_info(
                1,                                // waitSemaphoreCount
                submit_signal_semaphores,         // pWaitSemaphores
                1,                                // swapchainCount
                swapchains,                       // pSwapchains
                &swapchain.current_element_index, // pImageIndices
                nullptr                           // pResults
            );

            try
            {
                device_instance.queue.present().presentKHR( present_info );
            }
            catch ( vk::OutOfDateKHRError out_of_date )
            {
                handle_out_of_date_swapchain();
            }

            // Advance to next frame in flight
            swapchain.synchronization.advance_frame();
        }
    }

#pragma endregion drawing

/* -------------------------------------------------------------------------- */
/*                                  Clean Up                                  */
/* -------------------------------------------------------------------------- */
#pragma region clean_up
    void window_surface::destroy_surface()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan window surface cleanup" );

        let& instance = device_instance.get_vulkan_instance();
        instance.destroySurfaceKHR( vulkan_surface );
    }

    void window_surface::destroy_swapchain()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan swapchain cleanup" );

        let& vulkan_device = device_instance.get_vulkan_device();
        vulkan_device.destroySwapchainKHR( swapchain.vulkan_swapchain );
    }

    void window_surface::destroy_render_pass()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan render pass cleanup" );

        let& vulkan_device = device_instance.get_vulkan_device();
        vulkan_device.destroyRenderPass( render_pass );
    }

    void window_surface::destroy_frame_buffers()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan swapchain element cleanup" );

        let& vulkan_device = device_instance.get_vulkan_device();
        foreach ( element : swapchain.elements )
        {
            vulkan_device.destroyFramebuffer( element.frame_buffer );
            vulkan_device.destroyImageView( element.image_view );
        }
    }

    void window_surface::destroy_synchronization()
    {
        let task = vulkan_log_verbose.track_scope( "Vulkan synchronization cleanup" );

        let& vulkan_device = device_instance.get_vulkan_device();
        foreach ( frame : swapchain.synchronization.frames )
        {
            vulkan_device.destroySemaphore( frame.image_available );
            vulkan_device.destroySemaphore( frame.render_finished );
            vulkan_device.destroyFence( frame.in_flight );
        }
    }

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
        device_instance.wait_for_idle();

        destroy_frame_buffers();
        destroy_render_pass();
        destroy_swapchain();

        create_swapchain();
        create_render_pass();
        create_frame_buffers();
    }
#pragma endregion clean_up

/* -------------------------------------------------------------------------- */
/*                         Swapchain Creation Helpers                         */
/* -------------------------------------------------------------------------- */
#pragma region helpers

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

#pragma endregion helpers
} // namespace rnjin::graphics::vulkan