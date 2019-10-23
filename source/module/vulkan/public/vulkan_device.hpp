/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <vulkan/vulkan.hpp>

#include "vulkan_api.hpp"

namespace rnjin::graphics::vulkan
{
    class window_surface;

    class device
    {
        public: // methods
        device( api& api_instance );
        ~device();

        void set_reference_surface( const window_surface& surface );
        void initialize();
        void clean_up();

        void wait_for_idle() const;

        public: // accessors
        const vk::Instance& get_vulkan_instance() const;
        let& get_vulkan_device get_value( vulkan_device );
        let& get_physical_device get_value( physical_device );

        private: // methods
        void find_queue_family_indices();

        private: // members
        api& api_instance;

        bool is_initialized;

        vk::Device vulkan_device;
        vk::PhysicalDevice physical_device;

        const vk::SurfaceKHR* reference_surface;

        public: // groups
        group
        {
            public: // accessors
            let& main get_value( main_pool );
            let& transfer get_value( transfer_pool );

            private: // members
            vk::CommandPool main_pool;
            vk::CommandPool transfer_pool;

            friend class device;
        }
        command_pool;

        group
        {
            public: // accessors
            let& graphics get_value( graphics_queue );
            let& present get_value( present_queue );
            let& compute get_value( compute_queue );

            private: // members
            vk::Queue graphics_queue;
            vk::Queue present_queue;
            vk::Queue compute_queue;

            public: // members
            group
            {
                public: // accessors
                let graphics get_value( graphics_index );
                let compute get_value( compute_index );
                let present get_value( present_index );

                private: // members
                int graphics_index;
                int compute_index;
                int present_index;

                friend class device;
            }
            family_index;

            friend class device;
        }
        queue;
    };
} // namespace rnjin::graphics::vulkan