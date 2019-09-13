/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "glfw.hpp"
#include "renderer.hpp"
#include "vulkan_api.hpp"

namespace rnjin::graphics::vulkan
{
    class api_internal;
    class renderer_internal;

    class renderer : public renderer_base
    {
        public:
        renderer( api& api_instance );
        ~renderer();

        void initialize();

        // note: this _should_ be a double-specialized template function, but this accomplishes the same thing
        void add_target( window<GLFW>& target );
        void render( const render_view& view );

        private:
        renderer_internal* internal;

        friend class renderer_internal;
        friend class api_internal;
        friend class api;
    };
} // namespace rnjin::graphics::vulkan