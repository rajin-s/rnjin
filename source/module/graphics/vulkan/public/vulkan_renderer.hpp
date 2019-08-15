/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "glfw.hpp"

#include "renderer.hpp"
#include "vulkan_api.hpp"

namespace rnjin
{
    namespace graphics
    {
        template <>
        class renderer<vulkan> : public renderer_base<vulkan>
        {
            public:
            renderer( vulkan& api );
            ~renderer();

            void initialize();

            // note: this _should_ be a double-specialized template function, but this accomplishes the same thing
            void add_target( window<GLFW>& target );
            void render( const render_view& view );

            private:
            // Internal structure that actually does API calls
            // note: separated to prevent general engine code from needing to include / being able to access the actual Vulkan library (PIMPL)
            class internal;
            internal* _internal;

            friend class vulkan;
        };
    } // namespace graphics
} // namespace rnjin