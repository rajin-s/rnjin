/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_api_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        /* *** *** *** *
         * Private API *
         * *** *** *** */

        void vulkan::internal::initialize( const string& application_name )
        {
            initialize_instance( application_name );
            initialize_validation();
        }
        void vulkan::internal::clean_up()
        {
            clean_up_validation();
            clean_up_instance();
        }

        void vulkan::internal::initialize_instance( const string& application_name )
        {
            check_error_condition( return, vulkan_log_errors, instance.initialized == true, "Vulkan instance already initialized" );

            tracked_subregion( vulkan_log_verbose, "Vulkan instance initialization" )
            {
                let application_info     = get_application_info( application_name );
                let enabled_layer_names  = get_supported_target_layers();
                let enabled_extensions   = get_required_extensions( true, validation.any_enabled() );
                let debug_messenger_info = get_debug_messenger_create_info();

                const char** enabled_layers = new const char*[enabled_layer_names.size()];
                for ( uint i : range( enabled_layer_names.size() ) )
                {
                    enabled_layers[i] = enabled_layer_names[i].c_str();
                }

                nonconst auto instance_info = vk::InstanceCreateInfo(
                    {},                        // flags
                    &application_info,         // pApplicationInfo
                    0,                         // enabledLayerCount
                    nullptr,                   // ppEnabledLayerNames
                    enabled_extensions.size(), // enabledExtensionCount
                    enabled_extensions.data()  // ppEnabledExtensionNames
                );

                // Enable validation layers and debug callbacks for creation / destruction
                if ( validation.any_enabled() )
                {
                    instance_info.setEnabledLayerCount( enabled_layer_names.size() );
                    instance_info.setPpEnabledLayerNames( enabled_layers );
                    instance_info.setPNext( &debug_messenger_info );
                }

                instance.vulkan_instance = vk::createInstance( instance_info );
                vulkan_log_verbose.print_additional( "Using \1 instance extension(s)", enabled_extensions.size() );

                instance.dispatch_loader.init( instance.vulkan_instance );
                vulkan_log_verbose.print_additional( "Created dispatch loader dynamic" );

                instance.initialized = true;
            }
        }
        void vulkan::internal::clean_up_instance()
        {
            if ( instance.initialized )
            {
                instance.vulkan_instance.destroy();
                vulkan_log_verbose.print( "Destroyed Vulkan instance" );
            }
            else
            {
                vulkan_log_verbose.print_warning( "Cleaning up uninitialized Vulkan instance" );
            }

            instance.initialized = false;
        }

        void vulkan::internal::initialize_validation()
        {
            check_error_condition( return, vulkan_log_errors, validation.initialized == true, "Vulkan validation already initialized" );

            tracked_subregion( vulkan_log_verbose, "Vulkan validation initialization" );
            {
                if ( validation.any_enabled() )
                {
                    let debug_messenger_info = get_debug_messenger_create_info();

                    validation.debug_messenger = instance.vulkan_instance.createDebugUtilsMessengerEXT( debug_messenger_info, nullptr, instance.dispatch_loader );
                    validation.initialized     = true;

                    vulkan_log_verbose.print_additional( "Initialized Vulkan debug messenger" );
                }
                else
                {
                    validation.initialized = false;
                    vulkan_log_verbose.print_additional( "Vulkan validation disabled" );
                }
            }
        }
        void vulkan::internal::clean_up_validation()
        {
            if ( validation.initialized )
            {
                instance.vulkan_instance.destroyDebugUtilsMessengerEXT( validation.debug_messenger, nullptr, instance.dispatch_loader );
                vulkan_log_verbose.print( "Cleaned up Vulkan validation" );
            }

            validation.initialized = false;
        }

        // Used for instance creation / destruction, as well as debug messenger creation
        const vk::DebugUtilsMessengerCreateInfoEXT vulkan::internal::get_debug_messenger_create_info()
        {
            vk::DebugUtilsMessageSeverityFlagsEXT allowed_severity = {};

            if ( validation.enable_errors )
            {
                allowed_severity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            }
            if ( validation.enable_errors )
            {
                allowed_severity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
            }
            if ( validation.enable_errors )
            {
                allowed_severity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
            }

            vk::DebugUtilsMessageTypeFlagsEXT allowed_message_types =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;

            return vk::DebugUtilsMessengerCreateInfoEXT(
                {},                                // flags
                allowed_severity,                  // messageSeverity
                allowed_message_types,             // messageType
                &vulkan::internal::debug_callback, // pfnUserCallback
                (void*) this                       // pUserData
            );
        }

        const vk::ApplicationInfo get_application_info( const string& application_name )
        {
            // to do: query this info from somewhere else, rather than passing it in / having it be static
            return vk::ApplicationInfo(
                application_name.c_str(),   // pApplicationName
                VK_MAKE_VERSION( 0, 0, 0 ), // applicationVersion
                "rnjin",                    // pEngineName
                VK_MAKE_VERSION( 0, 0, 1 ), // engineVersion
                VK_API_VERSION_1_0          // apiVersion
            );
        }

        const list<string> get_supported_target_layers()
        {
            static const set<string> target_layers = {
                // "VK_LAYER_NV_optimus",
                // "VK_LAYER_VALVE_steam_overlay",
                // "VK_LAYER_VALVE_steam_fossilize",
                // "VK_LAYER_LUNARG_api_dump",
                // "VK_LAYER_LUNARG_assistant_layer",
                "VK_LAYER_LUNARG_core_validation",
                // "VK_LAYER_LUNARG_device_simulation",
                // "VK_LAYER_KHRONOS_validation",
                // "VK_LAYER_LUNARG_monitor",
                "VK_LAYER_LUNARG_object_tracker",
                // "VK_LAYER_LUNARG_screenshot",
                // "VK_LAYER_LUNARG_standard_validation",
                "VK_LAYER_LUNARG_parameter_validation",
                // "VK_LAYER_GOOGLE_threading",
                // "VK_LAYER_GOOGLE_unique_objects",
                // "VK_LAYER_LUNARG_vktrace",
            };

            list<string> supported_target_layers;

            list<vk::LayerProperties> supported_layers = vk::enumerateInstanceLayerProperties();

            vulkan_log_verbose.print( "Supported Vulkan validation layers:" );
            foreach ( layer : supported_layers )
            {
                const string layer_name( layer.layerName );
                bool is_target_layer = false;

                if ( target_layers.count( layer_name ) )
                {
                    is_target_layer = true;
                    supported_target_layers.push_back( layer_name );
                }

                vulkan_log_verbose.print_additional( "\1 \2", is_target_layer ? "=>" : "  ", layer_name );
            }

            return supported_target_layers;
        }

        const list<const char*> get_required_extensions( const bool require_present, const bool require_validation )
        {
            list<const char*> result;

            if ( require_present )
            {
                // Get extension information from GLFW
                const char** glfw_extensions;
                uint glfw_extension_count = 0;

                glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );
                check_error_condition( return result, vulkan_log_errors, glfw_extensions == NULL, "Vulkan is not supported on this system" );

                result.resize( glfw_extension_count );
                for ( uint i : range( glfw_extension_count ) )
                {
                    result[i] = glfw_extensions[i];
                }
            }

            if ( require_validation )
            {
                static const char* const validation_extension = "VK_EXT_debug_utils";
                result.push_back( validation_extension );
            }

            return result;
        }

        // Validation layer message callback
        VKAPI_ATTR VkBool32 VKAPI_CALL
        vulkan::internal::debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data )
        {
            vulkan::internal* self = (vulkan::internal*) user_data;

            if ( ( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) and self->validation.enable_errors )
            {
                vulkan_log_errors.print_error( "(Vulkan Error) \1", callback_data->pMessage );
            }
            else if ( ( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) and self->validation.enable_warnings )
            {
                vulkan_log_verbose.print_warning( "(Vulkan Warning) \1", callback_data->pMessage );
            }
            else if ( self->validation.enable_verbose )
            {
                vulkan_log_verbose.print( "(Vulkan) \1", callback_data->pMessage );
            }

            return VK_FALSE;
        }
    } // namespace graphics
} // namespace rnjin