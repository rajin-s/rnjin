/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

// #define GLFW_INCLUDE_VULKAN // in case this is included before any of the vulkan includes
#include <glfw3.h>
#include "graphics.hpp"

namespace rnjin
{
    namespace graphics
    {
        struct GLFW
        {
            public:
            typedef GLFWwindow window_type;

            static void initialize();
            static void clean_up();
            static void create_window( window<GLFW>& target );
            static void destroy_window( window<GLFW>& target );
            static void show_window( window<GLFW>& target );
            static void hide_window( window<GLFW>& target );
            static void close_window( window<GLFW>& target );
            static bool can_resize( window<GLFW>& target );
            static void resize_window( window<GLFW>& target, int2 size );
            static void on_glfw_window_resized( GLFWwindow* glfw_window, int width, int height );
            static void set_window_title( window<GLFW>& target, const string& title );

            private:
            static int window_count;
        };
    } // namespace graphics
} // namespace rnjin
