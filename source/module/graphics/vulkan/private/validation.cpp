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
            // Helper functions for creating and destroying debug messengers
            VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger );
            void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator );

            // Mark that validation layers should be used
            // note: should be called before instance::initialize() and validation::initialize()
            void internal::validation::enable()
            {
                enabled = true;
            }

            // Check if validation layers should be used
            const bool internal::validation::is_enabled() const
            {
                return enabled;
            }

            // Create the debug messenger
            // note: most initialization done in instance::initialize()
            //       maybe split that out and put it here?
            void internal::validation::initialize( const instance& reference_instance )
            {
                graphics_log.print( "Starting Vulkan validation layer initialization...", log_channel_vulkan );

                if (enabled)
                {
                    VkDebugUtilsMessengerCreateInfoEXT create_info = get_debug_messenger_create_info();
                    CreateDebugUtilsMessengerEXT( reference_instance.get_vulkan_instance(), &create_info, nullptr, &debug_messenger );

                    initialized = true;
                }
                else
                {
                    graphics_log.print( "Validation layers disabled", log_channel_vulkan );
                }
            }

            // Destroy the debug messenger if it was created
            void internal::validation::clean_up( const instance& reference_instance )
            {
                if (initialized)
                {
                    DestroyDebugUtilsMessengerEXT( reference_instance.get_vulkan_instance(), debug_messenger, nullptr );
                    debug_messenger = VK_NULL_HANDLE;
                    initialized = false;
                }

                graphics_log.print( "Finished Vulkan validation layer cleanup", log_channel_vulkan );
            }

            // Get the names of validation layers that are desired and supported on the current system
            const list<string> internal::validation::get_supported_target_layers() const
            {
                static const set<string> target_validation_layers = {
                    "VK_LAYER_KHRONOS_validation",
                };

                graphics_log << log::use_channel( log_channel_vulkan );
                list<string> supported_target_layers;

                // Get system-wide supported validation layers
                uint validation_layer_count = 0;
                vkEnumerateInstanceLayerProperties( &validation_layer_count, nullptr );
                list<VkLayerProperties> available_validation_layers( validation_layer_count );
                vkEnumerateInstanceLayerProperties( &validation_layer_count, available_validation_layers.data() );

                graphics_log.printf( "\1 supported validation layers:", { s( available_validation_layers.size() ) } );
                foreach ( layer_properties : available_validation_layers )
                {
                    const string layer_name( layer_properties.layerName );
                    graphics_log << log::indent();

                    if ( target_validation_layers.count( layer_name ) > 0 )
                    {
                        supported_target_layers.push_back( layer_name );
                        graphics_log << "*";
                    }
                    else
                    {
                        graphics_log << " ";
                    }

                    graphics_log << layer_name << log::line();
                }

                return supported_target_layers;
            }

            // Validation layer message callback
            static VKAPI_ATTR VkBool32 VKAPI_CALL validation_layer_debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                                                   VkDebugUtilsMessageTypeFlagsEXT type,
                                                                                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                                   void* user_data )
            {
                graphics_log.printf( "(Vulkan) \1", { string( callback_data->pMessage ) } );
                return VK_FALSE;
            }

            // Get the needed info to create a debug messenger
            // note: used by instance::initialize() to debug instance creation and destruction
            const VkDebugUtilsMessengerCreateInfoEXT internal::validation::get_debug_messenger_create_info() const
            {
                VkDebugUtilsMessengerCreateInfoEXT create_info = {};

                create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                create_info.pfnUserCallback = validation_layer_debug_callback;
                create_info.pUserData       = nullptr;

                return create_info;
            }

            // Load extension function for creating debug messengers
            VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger )
            {
                auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
                if ( function != nullptr )
                {
                    return function( instance, pCreateInfo, pAllocator, pDebugMessenger );
                }
                else
                {
                    return VK_ERROR_EXTENSION_NOT_PRESENT;
                }
            }

            // Load extension function for destroying debug messengers
            void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator )
            {
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
                if ( func != nullptr )
                {
                    func( instance, messenger, pAllocator );
                }
            }

        } // namespace vulkan

    } // namespace graphics
} // namespace rnjin