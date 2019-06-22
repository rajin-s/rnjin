/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "glfw.hpp"

namespace rnjin
{
    namespace graphics
    {
        int GLFW::window_count = 0;

        void GLFW::initialize()
        {
            glfwInit();
        }
        void GLFW::clean_up()
        {
            glfwTerminate();
        }

        void GLFW::create_window( window<GLFW>& target )
        {
            GLFW::window_count++;
            if ( GLFW::window_count == 1 )
            {
                initialize();
            }

            glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
            glfwWindowHint( GLFW_RESIZABLE, target.resizable ? GLFW_TRUE : GLFW_FALSE );
            target.api_window = glfwCreateWindow( target.size.x, target.size.y, target.title.c_str(), nullptr, nullptr );
        }
        void GLFW::destroy_window( window<GLFW>& target )
        {
            glfwDestroyWindow( target.api_window );
            GLFW::window_count--;

            if ( GLFW::window_count <= 0 )
            {
                clean_up();
            }
        }

        void GLFW::show_window( window<GLFW>& target )
        {
            glfwShowWindow( target.api_window );
        }
        void GLFW::hide_window( window<GLFW>& target )
        {
            glfwHideWindow( target.api_window );
        }
        void GLFW::close_window( window<GLFW>& target )
        {
            target.valid = false;
            glfwSetWindowShouldClose( target.api_window, 1 );
        }

        bool GLFW::can_resize( window<GLFW>& target )
        {
            return true;
        }

        void GLFW::resize_window( window<GLFW>& target, int2 size )
        {
            glfwSetWindowSize( target.api_window, size.x, size.y );
        }

        void GLFW::set_window_title( window<GLFW>& target, const string& title )
        {
            glfwSetWindowTitle( target.api_window, title.c_str() );
        }
    } // namespace graphics
} // namespace rnjin
