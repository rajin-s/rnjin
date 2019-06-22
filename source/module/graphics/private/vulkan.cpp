/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <graphics.hpp>

namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            namespace validation
            {
                void initialize();
                void clean_up();
                list<string> get_supported_validation_layers();
                bool using_validation_layers();
                VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info();
            } // namespace validation

            namespace device
            {
                VkPhysicalDevice physical_device = VK_NULL_HANDLE;
                VkDevice logical_device          = VK_NULL_HANDLE;
                VkQueue graphics_queue           = VK_NULL_HANDLE;
                VkQueue compute_queue            = VK_NULL_HANDLE;

                void initialize();
                void clean_up();
            } // namespace device

            VkInstance instance;
            const unsigned int log_channel_vulkan = 2;

            // Generate a Vulkan application info struct
            VkApplicationInfo get_application_info()
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

            // Get extensions required by windowing and validation layers
            list<const char*> get_required_extensions()
            {
                // Get extension information from GLFW
                const char** glfw_extensions;
                unsigned int glfw_extension_count = 0;
                glfw_extensions                   = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

                list<const char*> result( glfw_extensions, glfw_extensions + glfw_extension_count );

                if ( validation::using_validation_layers() )
                {
                    result.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
                }

                return result;
            }


            // Setup Vulkan
            void create_instance()
            {
                // Set up instance creation info struct
                graphics_log.print( "Get app info", log_channel_vulkan );
                VkApplicationInfo app_info       = get_application_info();
                VkInstanceCreateInfo create_info = {};
                create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                create_info.pApplicationInfo     = &app_info;

                // Get required extensions and include them
                graphics_log.print( "Get required extensions", log_channel_vulkan );
                list<const char*> required_extensions = get_required_extensions();
                create_info.enabledExtensionCount     = static_cast<uint32_t>( required_extensions.size() );
                create_info.ppEnabledExtensionNames   = required_extensions.data();

                graphics_log.printf( "Requires \1 extensions:", { s( required_extensions.size() ) }, log_channel_vulkan );
                foreach ( ext : required_extensions )
                {
                    graphics_log << "    " << string( ext ) << log::line();
                }

                // Declared here to not go out of scope be the time the instance is cretaed
                list<string> validation_layer_names;
                const char** validation_layers;
                VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info;

                // Enable validation layers for debug builds
                if ( validation::using_validation_layers() )
                {
                    graphics_log.print( "Get supported validation layers", log_channel_vulkan );
                    validation_layer_names              = validation::get_supported_validation_layers();
                    unsigned int validation_layer_count = validation_layer_names.size();

                    validation_layers = new const char*[validation_layer_count];
                    for ( int i = 0; i < validation_layer_count; i++ )
                    {
                        validation_layers[i] = validation_layer_names[i].c_str();
                    }

                    create_info.enabledLayerCount   = static_cast<uint32_t>( validation_layer_count );
                    create_info.ppEnabledLayerNames = validation_layers;

                    // show debug output for instance creation/destruction
                    debug_messenger_create_info = validation::get_debug_messenger_create_info();
                    create_info.pNext           = &debug_messenger_create_info;
                    graphics_log.printf( "Using \1 validation layers:", { s( validation_layer_names.size() ) } );
                    foreach ( layer : validation_layer_names )
                    {
                        graphics_log << "    " << string( layer ) << log::line();
                    }
                }
                else
                {
                    create_info.enabledLayerCount = 0;
                    graphics_log.print( "Using no validation layers" );
                }

                // Create the instance
                graphics_log.print( "Creating instance...", log_channel_vulkan );
                if ( vkCreateInstance( &create_info, nullptr, &instance ) )
                {
                    throw std::runtime_error( "Failed to create Vulkan instance!" );
                }

                graphics_log.print( "Vulkan instance created", log_channel_vulkan );

                // free memory used by validation layer array (only needed for initialization)
                delete validation_layers;

                // Get supported extension information from vulkan
                // unsigned int vulkan_extension_count = 0;
                // vkEnumerateInstanceExtensionProperties( nullptr, &vulkan_extension_count, nullptr );
                // list<VkExtensionProperties> vulkan_extensions( vulkan_extension_count );
                // vkEnumerateInstanceExtensionProperties( nullptr, &vulkan_extension_count, vulkan_extensions.data() );

                // graphics_log << log::use_channel( log_channel_vulkan );
                // graphics_log << log::start() << "Vulkan supported extensions are:" << log::line();
                // for ( const auto& extension : vulkan_extensions )
                // {
                //     graphics_log << "    " << extension.extensionName << "\n";
                // }
            }

            // Clean up Vulkan when done
            void destroy_instance()
            {
                vkDestroyInstance( instance, nullptr );
            }

            // Public API
            void initialize()
            {
                create_instance();
                if ( validation::using_validation_layers() )
                {
                    validation::initialize();
                }
                device::initialize();
            }
            void clean_up()
            {
                validation::clean_up();
                device::clean_up();
                destroy_instance();
            }

            namespace validation
            {
                VkDebugUtilsMessengerEXT debug_messenger;
                bool using_debug_messenger = false;

                const set<string> target_validation_layers = {
                    "VK_LAYER_KHRONOS_validation",
                };

                bool using_validation_layers()
                {
                    min_debug( debug )
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }

                // Validation layer message callback
                static VKAPI_ATTR VkBool32 VKAPI_CALL validation_layer_debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                                                       VkDebugUtilsMessageTypeFlagsEXT type,
                                                                                       const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                                       void* user_data )
                {
                    graphics_log.printf( "[Vulkan] \1", { string( callback_data->pMessage ) } );
                    return VK_FALSE;
                }

                // Get target validation layers that are supported
                list<string> get_supported_validation_layers()
                {
                    list<string> result;

                    uint32_t validation_layer_count = 0;
                    vkEnumerateInstanceLayerProperties( &validation_layer_count, nullptr );
                    list<VkLayerProperties> available_validation_layers( validation_layer_count );
                    vkEnumerateInstanceLayerProperties( &validation_layer_count, available_validation_layers.data() );

                    graphics_log.printf( "\1 supported validation layers:", { s( available_validation_layers.size() ) } );
                    foreach ( layer_properties : available_validation_layers )
                    {
                        const string layer_name( layer_properties.layerName );
                        graphics_log << "    " << layer_name;
                        if ( target_validation_layers.count( layer_name ) > 0 )
                        {
                            graphics_log << " (*)";
                            result.push_back( layer_name );
                        }
                        graphics_log << log::line();
                    }

                    return result;
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

                // Create the debug messenger to be used with validation layers
                VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info()
                {
                    VkDebugUtilsMessengerCreateInfoEXT create_info = {};

                    create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    create_info.pfnUserCallback = validation_layer_debug_callback;
                    create_info.pUserData       = nullptr;

                    return create_info;
                }

                void initialize()
                {
                    using_debug_messenger = true;

                    VkDebugUtilsMessengerCreateInfoEXT create_info = get_debug_messenger_create_info();
                    CreateDebugUtilsMessengerEXT( instance, &create_info, nullptr, &debug_messenger );
                }
                void clean_up()
                {
                    if ( using_debug_messenger )
                    {
                        using_debug_messenger = false;

                        DestroyDebugUtilsMessengerEXT( instance, debug_messenger, nullptr );
                    }
                }
            } // namespace validation

            namespace device
            {
                struct queue_family_indices
                {
                    uint32_t graphics;
                    uint32_t compute;

                    queue_family_indices() : graphics( -1 ), compute( -1 ) {}

                    bool is_complete()
                    {
                        return graphics >= 0 && compute >= 0;
                    }
                };

                queue_family_indices get_supported_queue_families( VkPhysicalDevice device )
                {
                    // Check what queue families are supported
                    uint32_t queue_family_count = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties( device, &queue_family_count, nullptr );
                    list<VkQueueFamilyProperties> queue_families( queue_family_count );
                    vkGetPhysicalDeviceQueueFamilyProperties( device, &queue_family_count, queue_families.data() );

                    queue_family_indices indices;

                    uint32_t i = 0;
                    foreach ( queue_family : queue_families )
                    {
                        if ( queue_family.queueCount > 0 )
                        {
                            if ( queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT )
                            {
                                indices.graphics = i;
                            }
                            if ( queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT )
                            {
                                indices.compute = i;
                            }

                            if ( indices.is_complete() )
                            {
                                break;
                            }
                        }
                        i++;
                    }
                    return indices;
                }

                int get_device_suitability( VkPhysicalDevice device )
                {
                    int score = 0;

                    // Check device properties and features
                    VkPhysicalDeviceProperties device_properties;
                    VkPhysicalDeviceFeatures device_features;

                    vkGetPhysicalDeviceProperties( device, &device_properties );
                    vkGetPhysicalDeviceFeatures( device, &device_features );

                    if ( device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) score += 1000;
                    if ( device_features.geometryShader ) score += 200;
                    if ( device_features.tessellationShader ) score += 200;

                    queue_family_indices queue_families = get_supported_queue_families( device );
                    if ( queue_families.graphics >= 0 )
                    {
                        score += 10000;
                    }
                    if ( queue_families.compute >= 0 )
                    {
                        score += 100;
                    }

                    graphics_log << "    " << string( device_properties.deviceName ) << " (vendor:" << s( device_properties.vendorID ) << ") score: " << s( score ) << log::line();
                    return score;
                }

                void pick_physical_device()
                {
                    uint32_t device_count = 0;
                    vkEnumeratePhysicalDevices( instance, &device_count, nullptr );
                    if ( device_count == 0 )
                    {
                        throw std::runtime_error( "Failed to find GPU with Vulkan support!" );
                    }
                    list<VkPhysicalDevice> devices( device_count );
                    vkEnumeratePhysicalDevices( instance, &device_count, devices.data() );

                    int max_suitability          = -1;
                    VkPhysicalDevice best_device = VK_NULL_HANDLE;

                    graphics_log.printf( "Found \1 Vulkan-compatible devices", { s( device_count ) }, log_channel_vulkan );
                    foreach ( device : devices )
                    {
                        int suitability = get_device_suitability( device );
                        if ( suitability > max_suitability )
                        {
                            best_device     = device;
                            max_suitability = suitability;
                        }
                    }

                    physical_device = best_device;

                    if ( physical_device == VK_NULL_HANDLE )
                    {
                        throw std::runtime_error( "Failed to find a suitable GPU!" );
                    }
                }

                void create_logical_device()
                {
                    // Get queue creation info
                    const float queue_priority = 1.0;
                    list<VkDeviceQueueCreateInfo> queue_create_infos;

                    queue_family_indices queue_families = get_supported_queue_families( physical_device );
                    if ( queue_families.graphics >= 0 )
                    {
                        VkDeviceQueueCreateInfo queue_create_info = {};
                        queue_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                        queue_create_info.queueFamilyIndex        = queue_families.graphics;
                        queue_create_info.queueCount              = 1;
                        queue_create_info.pQueuePriorities        = &queue_priority;

                        queue_create_infos.push_back( queue_create_info );
                    }
                    if ( queue_families.compute >= 0 )
                    {
                        VkDeviceQueueCreateInfo queue_create_info = {};
                        queue_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                        queue_create_info.queueFamilyIndex        = queue_families.compute;
                        queue_create_info.queueCount              = 1;
                        queue_create_info.pQueuePriorities        = &queue_priority;

                        queue_create_infos.push_back( queue_create_info );
                    }

                    // Specify required device features
                    VkPhysicalDeviceFeatures device_features = {};

                    // Create logical device
                    VkDeviceCreateInfo create_info = {};
                    create_info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

                    create_info.queueCreateInfoCount = queue_create_infos.size();
                    create_info.pQueueCreateInfos    = queue_create_infos.data();
                    create_info.pEnabledFeatures     = &device_features;

                    // Specify validation layers to use (only needed to support older Vulkan implementations)
                    list<const char*> validation_layers;
                    if ( validation::using_validation_layers() )
                    {
                        list<string> validation_layer_names = validation::get_supported_validation_layers();
                        foreach ( name : validation_layer_names )
                        {
                            validation_layers.push_back( name.c_str() );
                        }

                        create_info.enabledLayerCount   = validation_layers.size();
                        create_info.ppEnabledLayerNames = validation_layers.data();
                    }
                    else
                    {
                        create_info.enabledLayerCount = 0;
                    }

                    // Specify device-specific extensions to use
                    create_info.enabledExtensionCount = 0;

                    if ( vkCreateDevice( physical_device, &create_info, nullptr, &logical_device ) != VK_SUCCESS )
                    {
                        throw std::runtime_error( "Failed to create Vulkan logical device!" );
                    }

                    // Get queue handles
                    if ( queue_families.graphics >= 0 )
                    {
                        vkGetDeviceQueue( logical_device, queue_families.graphics, 0, &graphics_queue );
                    }
                    if ( queue_families.compute >= 0 )
                    {
                        vkGetDeviceQueue( logical_device, queue_families.compute, 0, &compute_queue );
                    }
                }

                void initialize()
                {
                    pick_physical_device();
                    create_logical_device();
                }

                void clean_up()
                {
                    vkDestroyDevice( logical_device, nullptr );
                }
            } // namespace device
        }     // namespace vulkan
    }         // namespace graphics
} // namespace rnjin