/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_renderer.hpp"
#include "vulkan_renderer_internal.hpp"

namespace rnjin::graphics::vulkan
{
    /// Public API ///
    renderer::renderer( api& api_instance ) : renderer_base()
    {
        internal = new renderer_internal( api_instance );
    }
    renderer::~renderer()
    {
        internal->clean_up();
        delete internal;
    }

    void renderer::initialize()
    {
        // graphics_log.print( "Initialize Vulkan Renderer" );
        internal->initialize();
    }

    void renderer::add_target( window<GLFW>& target )
    {
        vulkan_log_verbose.print( "Bind GLFW window to Vulkan renderer" );
        internal->add_window_target( target );
    }

    void renderer::render( const render_view& view )
    {
        internal->render( view );
    }
} // namespace rnjin::graphics::vulkan