/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            const VkApplicationInfo get_application_info();
            list<const char*> get_required_extensions();

            // Create the vulkan instance
            // note: reference_validation is not initialized yet
            void internal::instance::initialize( const validation& reference_validation )
            {
                graphics_log << log::use_channel( log_channel_vulkan );
                graphics_log.print( "Starting Vulkan instance creation...", log_channel_vulkan );

                VkInstanceCreateInfo create_info = {};
                create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

                // Fill in basic info
                VkApplicationInfo app_info;
                graphics_log.print( "Get application info..." ), log_channel_vulkan;
                {
                    app_info                     = get_application_info();
                    create_info.pApplicationInfo = &app_info;
                }

                // Fill in extension info
                list<const char*> required_extensions;
                graphics_log.print( "Get required extensions..." ), log_channel_vulkan;
                {
                    required_extensions = get_required_extensions();
                    if ( reference_validation.is_enabled() )
                    {
                        required_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
                    }

                    create_info.enabledExtensionCount   = required_extensions.size();
                    create_info.ppEnabledExtensionNames = required_extensions.data();

                    graphics_log.printf( "Requires \1 extensions:", { s( required_extensions.size() ) }, log_channel_vulkan );

                    foreach ( extension_name : required_extensions )
                    {
                        graphics_log << log::indent() << string( extension_name ) << log::line();
                    }
                }

                // Fill in validation layer info
                list<string> validation_layer_names;
                const char** validation_layers = nullptr;
                VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info;
                graphics_log.print( "Get active validation layers..." ), log_channel_vulkan;
                {
                    if ( reference_validation.is_enabled() )
                    {
                        validation_layer_names            = reference_validation.get_supported_target_layers();
                        const uint validation_layer_count = validation_layer_names.size();

                        // Get char array pointers from strings
                        validation_layers = new const char*[validation_layer_count];
                        for ( uint i : range( 0, validation_layer_count ) )
                        {
                            validation_layers[i] = validation_layer_names[i].c_str();
                        }

                        create_info.enabledLayerCount   = (uint)validation_layer_names.size();
                        create_info.ppEnabledLayerNames = validation_layers;

                        // Create debug messenger for instance creation and deletion
                        debug_messenger_create_info = reference_validation.get_debug_messenger_create_info();
                        create_info.pNext           = &debug_messenger_create_info;

                        // Report used layers
                        graphics_log.printf( "Using \1 validation layers", { s( validation_layer_count ) } );
                        foreach ( layer_name : validation_layer_names )
                        {
                            graphics_log << log::indent() << layer_name << log::line();
                        }
                    }
                    else
                    {
                        create_info.enabledLayerCount   = 0;
                        create_info.ppEnabledLayerNames = nullptr;

                        graphics_log.print( "Validation layers disabled", log_channel_vulkan );
                    }
                }

                // Create the Vulkan instance
                VkResult create_result = vkCreateInstance( &create_info, nullptr, &vulkan_instance );

                // Free specially allocated memory
                if ( validation_layers != nullptr )
                {
                    delete validation_layers;
                }

                // Check if creation failed
                if ( create_result != VK_SUCCESS )
                {
                    throw std::runtime_error( "Failed to create Vulkan instance!" );
                }

                graphics_log.print( "Finished Vulkan instance creation", log_channel_vulkan );
            }

            // Destroy the vulkan instance
            void internal::instance::clean_up()
            {
                vkDestroyInstance( vulkan_instance, nullptr );
                {
                    vulkan_instance = VK_NULL_HANDLE;
                }
                graphics_log.print( "Finished Vulkan instance cleanup", log_channel_vulkan );
            }

            const VkInstance& internal::instance::get_vulkan_instance() const
            {
                return vulkan_instance;
            }

            const VkApplicationInfo get_application_info()
            {
                VkApplicationInfo info  = {};
                info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                info.pApplicationName   = "rnjin";
                info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
                info.pEngineName        = "No Engine";
                info.engineVersion      = VK_MAKE_VERSION( 0, 0, 0 );
                info.apiVersion         = VK_API_VERSION_1_0;
                return info;
            }

            list<const char*> get_required_extensions()
            {
                // Get extension information from GLFW
                const char** glfw_extensions;
                unsigned int glfw_extension_count = 0;

                glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );
                list<const char*> result( glfw_extensions, glfw_extensions + glfw_extension_count );

                return result;
            }
        } // namespace vulkan

    } // namespace graphics
} // namespace rnjin