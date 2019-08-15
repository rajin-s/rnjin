/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_renderer.hpp"
#include "vulkan_renderer_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        /// Public API ///
        renderer<vulkan>::renderer( vulkan& api ) : renderer_base( api )
        {
            _internal = new internal( api );
        }
        renderer<vulkan>::~renderer()
        {
            _internal->clean_up();
            delete _internal;
        }

        void renderer<vulkan>::initialize()
        {
            // graphics_log.print( "Initialize Vulkan Renderer" );
            _internal->initialize();
        }

        void renderer<vulkan>::add_target( window<GLFW>& target )
        {
            vulkan_log_verbose.print( "Bind GLFW window to Vulkan renderer" );
            _internal->add_window_target( target );
        }

        void renderer<vulkan>::render( const render_view& view )
        {
            _internal->render( view );
        }
    } // namespace graphics
} // namespace rnjin