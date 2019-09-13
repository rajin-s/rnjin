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

        for ( auto& window_target : window_targets )
        {
            window_target.render();
        }
    }

    // Window interaction
    void renderer_internal::add_window_target( window<GLFW>& target )
    {
        let task = vulkan_log_verbose.track_scope( "Adding window target to Vulkan renderer" );

        let next_index = window_targets.size();
        window_targets.emplace_back( *this );
        window_targets[next_index].create_surface( target );
    }
} // namespace rnjin::graphics::vulkan