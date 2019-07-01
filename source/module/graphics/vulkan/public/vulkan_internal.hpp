/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
// #include <vulkan/vulkan.h>

#include <core.hpp>
#include <glfw.hpp>
#include <graphics.hpp>


namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            extern const int log_channel_vulkan;

            class internal
            {
                public:
                void initialize( const window<GLFW>& target_window, const bool enable_validation );
                void clean_up();

                private:
                class device;
                class surface;
                class pipeline;
                class validation;

                class instance
                {
                    public:
                    void initialize( const validation& reference_validation );
                    void clean_up();

                    const VkInstance& get_vulkan_instance() const;

                    private:
                    VkInstance vulkan_instance;

                } _instance;

                class device
                {
                    public:
                    void initialize( const instance& reference_instance, const surface& reference_surface, const validation& reference_validation );
                    void clean_up();

                    const VkDevice get_vulkan_device() const;

                    struct swap_chain_support_details
                    {
                        VkSurfaceCapabilitiesKHR capabilities;
                        list<VkSurfaceFormatKHR> formats;
                        list<VkPresentModeKHR> present_modes;
                    };
                    const swap_chain_support_details get_swap_chain_support( const surface& reference_surface ) const;

                    struct queue_family_indices
                    {
                        int graphics;
                        int compute;
                        int present;

                        queue_family_indices() : graphics( -1 ), compute( -1 ), present( -1 ) {}

                        bool is_complete()
                        {
                            return graphics >= 0 && compute >= 0 && present >= 0;
                        }
                    };
                    const queue_family_indices get_queue_indices() const;

                    private:
                    VkPhysicalDevice physical;
                    VkDevice vulkan_device;

                    struct
                    {
                        VkQueue graphics;
                        VkQueue present;
                        VkQueue compute;

                        queue_family_indices indices;
                    } queue;

                    static const swap_chain_support_details get_swap_chain_support_details_for_device( const VkPhysicalDevice device, const VkSurfaceKHR surface );
                    static const int get_device_suitability( const VkPhysicalDevice, const VkSurfaceKHR surface );
                    static const queue_family_indices get_supported_queue_families_for_device( const VkPhysicalDevice device, const VkSurfaceKHR surface );

                } _device;

                class surface
                {
                    public:
                    void initialize( const instance& reference_instance, const window<GLFW>& target_window );
                    void clean_up( const instance& reference_instance, const device& reference_device );

                    void create_swap_chain( const device& reference_device );

                    const VkSurfaceKHR get_vulkan_surface() const;

                    private:
                    const window<GLFW>* glfw_window;
                    VkSurfaceKHR vulkan_surface;

                    VkSwapchainKHR swap_chain;
                    VkFormat swap_chain_image_format;
                    VkExtent2D swap_chain_extent;
                    list<VkImage> swap_chain_images;
                    list<VkImageView> swap_chain_image_views;

                    void create_image_views( const device& reference_device );

                } _surface;

                class pipeline
                {
                    public:
                    void initialize();
                    void clean_up();

                    private:
                } _pipeline;

                class validation
                {
                    public:
                    void initialize( const instance& reference_instance );
                    void clean_up( const instance& reference_instance );

                    void enable();
                    const bool is_enabled() const;

                    const list<string> get_supported_target_layers() const;
                    const VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info() const;

                    private:
                    bool initialized = false;
                    bool enabled     = false;
                    VkDebugUtilsMessengerEXT debug_messenger;

                } _validation;
            };
        } // namespace vulkan

    } // namespace graphics
} // namespace rnjin