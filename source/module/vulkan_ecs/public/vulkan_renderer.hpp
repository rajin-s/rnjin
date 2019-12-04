/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include third_party_library( "vulkan/vulkan.hpp" )

#include "vulkan_window_surface.hpp"
#include "vulkan_api.hpp"
#include "vulkan_resource_collectors.hpp"

#include rnjin_module( ecs )
#include rnjin_module( reflection )

namespace rnjin::graphics::vulkan
{
    class renderer                                //
      : public ecs::system<                       //
            read_from<model_resources>,           //
            read_from<mesh_resources::reference>, //
            read_from<material_resources::reference>>
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

        void update( entity_components& components ) override;

        void before_update() override;
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

namespace reflection
{
    auto_reflect_type( rnjin::graphics, vulkan::renderer );
} // namespace reflection