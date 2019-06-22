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
        // Graphics output log
        // ch.1 -> windowing
        extern log::source graphics_log;
        const unsigned int log_channel_window = 1;

        template <class API>
        // A window managed by an aribitrary windowing API (such as GLFW in glfw.hpp)
        class window
        {
            // The API can access private members
            friend API;

            public:
            // Create a window (without showing it)
            window( const string title, const int2 size, bool resizable ) : title( title ), size( size ), resizable( resizable ), valid( true ), shown( false )
            {
                graphics_log.printf( "Create window '\1' (\2 x \3px)", { title, s( size.x ), s( size.y ) }, log_channel_window );
                API::create_window( *this );
            }
            // Destroy a window
            ~window()
            {
                graphics_log.printf( "Destroy window '\1'", { this->title }, log_channel_window );
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
            bool can_resize()
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
            int2 get_size()
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
            const string& get_title()
            {
                return this->title;
            }

            // Get the API-specific window structure (for debugging)
            typename API::window_type* get_api_window()
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
