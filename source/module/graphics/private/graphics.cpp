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
        log::source graphics_log( "rnjin.graphics", log::write_to_file::immediately );

        // void window::initialize()
        // {
        //     glfwInit();
        // }

        // GLFWwindow* window;
        // VkInstance instance;

        // void init_window()
        // {
        //     glfwInit();
        //     glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
        //     glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
        //     window = glfwCreateWindow( 800, 800, "Vulkan", nullptr, nullptr );
        // }

        // void create_instance()
        // {
        //     VkApplicationInfo app_info  = {};
        //     app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        //     app_info.pApplicationName   = "Hello Triangle";
        //     app_info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
        //     app_info.pEngineName        = "No Engine";
        //     app_info.engineVersion      = VK_MAKE_VERSION( 0, 0, 0 );
        //     app_info.apiVersion         = VK_API_VERSION_1_0;

        //     VkInstanceCreateInfo create_info = {};
        //     create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        //     create_info.pApplicationInfo     = &app_info;

        //     unsigned int glfw_extension_count = 0;
        //     const char** glfw_extensions;
        //     glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

        //     create_info.enabledExtensionCount   = glfw_extension_count;
        //     create_info.ppEnabledExtensionNames = glfw_extensions;

        //     create_info.enabledLayerCount = 0;

        //     if ( vkCreateInstance( &create_info, nullptr, &instance ) )
        //     {
        //         throw std::runtime_error( "Failed to create Vulkan instance!" );
        //     }

        //     unsigned int extension_count = 0;
        //     vkEnumerateInstanceExtensionProperties( nullptr, &extension_count, nullptr );
        //     list<VkExtensionProperties> extensions( extension_count );
        //     vkEnumerateInstanceExtensionProperties( nullptr, &extension_count, extensions.data() );

        //     graphics_log.print( "Available extensions: " );
        //     for ( const auto& e : extensions )
        //     {
        //         graphics_log << "    " << e.extensionName << log::line();
        //     }
        // }

        // void init_vulkan()
        // {
        //     create_instance();
        // }

        // void main_loop()
        // {
        //     while ( !glfwWindowShouldClose( window ) )
        //     {
        //         glfwPollEvents();
        //     }
        // }

        // void cleanup()
        // {
        //     vkDestroyInstance( instance, nullptr );
        //     glfwDestroyWindow( window );
        //     glfwTerminate();
        // }

        // void test()
        // {
        //     init_window();
        //     init_vulkan();
        //     main_loop();
        //     cleanup();

        //     // glfwInit();
        //     // glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
        //     // GLFWwindow* window = glfwCreateWindow( 800, 800, "Vulkan Window", nullptr, nullptr );

        //     // unsigned int extensionCount = 0;
        //     // vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

        //     // log::main << s( extensionCount ) << " extensions supported" << log::line();

        //     // while ( !glfwWindowShouldClose( window ) )
        //     // {
        //     //     glfwPollEvents();
        //     // }

        //     // glfwDestroyWindow( window );
        //     // glfwTerminate();
        // }
    } // namespace graphics
} // namespace rnjin