/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <graphics.hpp>
#include <log.hpp>

namespace rnjin
{
    namespace graphics
    {
        void test()
        {
            glfwInit();
            glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
            GLFWwindow* window = glfwCreateWindow( 800, 800, "Vulkan Window", nullptr, nullptr );

            unsigned int extensionCount = 0;
            vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

            log::main << s( extensionCount ) << " extensions supported" << log::line();

            while ( !glfwWindowShouldClose( window ) )
            {
                glfwPollEvents();
            }

            glfwDestroyWindow( window );
            glfwTerminate();
        }
    } // namespace graphics
} // namespace rnjin