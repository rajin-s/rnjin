/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "log.hpp"

namespace rnjin
{
    namespace graphics
    {
        // Graphics logs flags used in masked log source creation
        enum class log_flag : uint
        {
            verbose = 2,
            errors  = 3,
            vulkan  = 4
        };

        // Graphics output log
        extern log::source& get_graphics_log();
        extern log::source::masked graphics_log_verbose;
        extern log::source::masked graphics_log_errors;

        // A window managed by an aribitrary windowing API (such as GLFW in glfw.hpp)
        template <class API>
        class window
        {
            // The API can access private members
            friend API;

            public:
            // Create a window (without showing it)
            window( const string title, const int2 size, bool resizable ) : title( title ), size( size ), resizable( resizable ), valid( true ), shown( false )
            {
                graphics_log_verbose.print( "Create window '\1' (\2 x \3px)", title, size.x, size.y );
                API::create_window( *this );
            }
            // Destroy a window
            ~window()
            {
                graphics_log_verbose.print( "Destroy window '\1'", this->title );
                valid = false;
                API::destroy_window( *this );
            }

            // Make a window visible
            void show()
            {
                if ( valid )
                {
                    this->shown = true;
                    API::show_window( *this );
                }
            }
            // Make a window not visible (but still valid)
            void hide()
            {
                if ( valid )
                {
                    this->shown = false;
                    API::hide_window( *this );
                }
            }
            // Close a window (may or may not be valid)
            void close()
            {
                if ( valid )
                {
                    this->shown = false;
                    API::close_window( *this );
                }
            }

            // Can this window's size be changed?
            bool can_resize() const
            {
                return resizable && API::can_resize( *this );
            }

            // Change this windows size (in pixels), if possible
            void resize( int2 size )
            {
                if ( can_resize() )
                {
                    API::resize_window( *this, size );
                }
            }
            // Get this windows size (in pixels)
            int2 get_size() const
            {
                return size;
            }

            // Set the title of this window
            void set_title( const string& title )
            {
                if ( valid )
                {
                    this->title = title;
                }
                API::set_window_title( *this, title );
            }
            // Get the title of this window
            const string& get_title() const
            {
                return this->title;
            }

            // Get the API-specific window structure (for debugging)
            typename API::window_type* get_api_window() const
            {
                return api_window;
            }

            private:
            typename API::window_type* api_window;
            const bool resizable;
            string title;
            bool valid;
            bool shown;
            int2 size;
        };
    } // namespace graphics
} // namespace rnjin
