/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "render_view.hpp"
#include "vulkan_window_surface.hpp"
#include "vulkan_renderer_internal.hpp"

namespace rnjin::graphics::vulkan
{
    void renderer_internal::render( const render_view& view )
    {
        static uint frame_number = 1;
        vulkan_log.print( "Render frame \1", frame_number++ );

        window_target.render( view );
    }

    // Window interaction
    void renderer_internal::register_window_target( window<GLFW>& target )
    {
        let task = vulkan_log_verbose.track_scope( "Registering window target with Vulkan renderer" );
        
        // Currently only one window surface per renderer is supported.
        // note: To support multiple windows, we would need to make sure the physical device
        //       supports presentation to all of their surfaces, and make sure all pipelines
        //       in the resource database are compatible with their image formats
        check_error_condition( return, vulkan_log_errors, window_target.get_vulkan_surface(), "Can't register multiple window targets to one renderer, ignoring surface for '\1'", target.get_title() );

        window_target.create_surface( target );
    }
} // namespace rnjin::graphics::vulkan