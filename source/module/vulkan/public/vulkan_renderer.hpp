/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include <vulkan/vulkan.hpp>

#include "ecs/public/ecs.hpp"
#include "vulkan_window_surface.hpp"
#include "vulkan_api.hpp"
#include "vulkan_resources.hpp"

namespace rnjin::graphics::vulkan
{
    class renderer : public ecs::system<read_from<internal_resources>>
    {
        public: // methods
        renderer( const device& device_instance, window_surface& target );
        ~renderer();

        void initialize();

        public: // accessors
        let& get_device get_value( device_instance );
        let& get_render_pass get_value( target.get_render_pass() );

        protected: // internal
        void define() override;
        void before_update() override;
        void update( entity_components& components ) override;
        void after_update() override;

        private: // members
        const device& device_instance;
        window_surface& target;

        usize frame_number;

        group
        {
            vk::CommandBuffer command_buffer;
        }
        current_frame;
    };
} // namespace rnjin::graphics::vulkan

/* -------------------------------------------------------------------------- */
/*                               Reflection Info                              */
/* -------------------------------------------------------------------------- */

reflection_info_for( rnjin::graphics::vulkan, renderer )
{
    reflect_type_name( "vulkan::renderer" );
};