/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "graphics.hpp"
#include "render_view.hpp"

namespace rnjin
{
    namespace graphics
    {
        class renderer_base
        {
            public:
            virtual void render( const render_view& view ) is_abstract;

            template <typename T>
            void add_target( T& target )
            {
                const bool is_invalid_target_type = true;
                check_error_condition( return, graphics_log_errors, is_invalid_target_type == true, "Renderer doesn't support given target type" );
            }
        };
    } // namespace graphics
} // namespace rnjin