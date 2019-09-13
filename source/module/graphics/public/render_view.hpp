/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "graphics.hpp"
#include "shader.hpp"

namespace rnjin
{
    namespace graphics
    {
        // The intermediate data format between logical and rendering parts of the engine
        // A render_view should be translatable to rendering commands for any API
        class render_view
        {
            private:
            struct item
            {
                const shader& vertex_shader;
                const shader& fragment_shader;

                const list<float3>& vertices;
                const list<int>& indices;
            };

            list<item> items;

            public:
            let& get_items get_value( items );
        };
    } // namespace graphics
} // namespace rnjin
