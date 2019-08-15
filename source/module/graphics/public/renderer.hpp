/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "graphics.hpp"
#include "render_view.hpp"

namespace rnjin
{
    namespace graphics
    {
        // Consistent interface for renderers that maintains basic state and forwards calls to API
        // note: API must inherit from (or have the same interface as) render_api
        template <typename API>
        class renderer_base
        {
            public:
            renderer_base( API& target_api ) : api( target_api )
            {
                graphics_log_verbose.print( "Constructing renderer (\1)", api.get_name() );
            }
            ~renderer_base()
            {
                graphics_log_verbose.print( "Destroying renderer (\1)", api.get_name() );
            }

            template <typename T>
            void add_target( T& target )
            {
                const bool is_invalid_target_type = true;
                check_error_condition( return, graphics_log_errors, is_invalid_target_type == true, "Renderer doesn't support given target type" );
            }

            virtual void render( const render_view& view ) is_abstract;

            protected:
            API& api;
        };

        // Generic renderer class (should only be specialized for different APIs, not interacted with directly)
        // note: other renderer<...> types inherir from render_base<...>, NOT renderer<...>
        template <typename API>
        class renderer : public renderer_base<API>
        {
            public:
            renderer( API& target_api ) : renderer_base( target_api )
            {
                const bool is_invalid_renderer_type = true;
                check_error_condition( return, graphics_log_errors, is_invalid_renderer_type == true, "Renderer constructed with invalid API template arument" );
            }
        };
    } // namespace graphics
} // namespace rnjin