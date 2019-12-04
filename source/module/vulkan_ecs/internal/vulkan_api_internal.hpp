/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include <vulkan/vulkan.hpp>

#include "vulkan_api.hpp"

namespace rnjin::graphics::vulkan
{
    class api_internal
    {
        public:
        void initialize( const string& application_name );
        void clean_up();

        void initialize_instance( const string& application_name );
        void clean_up_instance();

        void initialize_validation();
        void clean_up_validation();

        const vk::DebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info();

        group
        {
            bool initialized;

            vk::Instance vulkan_instance;
            vk::DispatchLoaderDynamic dispatch_loader;
        }
        instance;

        group
        {
            bool enable_errors;
            bool enable_warnings;
            bool enable_verbose;

            bool initialized;

            vk::DebugUtilsMessengerEXT debug_messenger;

            let any_enabled get_value( enable_errors or enable_warnings or enable_verbose );
        }
        validation;

        // Validation layer message callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data );
    };

    const vk::ApplicationInfo get_application_info( const string& application_name );
    const list<string> get_supported_target_layers();
    const list<const char*> get_required_extensions( const bool require_present, const bool require_validation );
} // namespace rnjin::graphics::vulkan