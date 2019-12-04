/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include <vulkan/vulkan.hpp>

#include "vulkan_memory.hpp"

#include "core/module.h"
#include "reflection/module.h"
#include "graphics/ecs.h"

namespace rnjin::graphics::vulkan
{
    /* -------------------------------------------------------------------------- */
    /*                                 Components                                 */
    /* -------------------------------------------------------------------------- */

    component_class( material_resources )
    {
        public: // methods
        material_resources();
        ~material_resources();

        public: // accessors
        let& get_pipeline get_value( pipeline );

        private: // members
        version_id current_material_version;
        version_id current_uniforms_version;

        render_pipeline pipeline;
        buffer_allocation uniform_buffer_allocation;

        friend class material_collector;
    };

    /* -------------------------------------------------------------------------- */
    /*                                   Systems                                  */
    /* -------------------------------------------------------------------------- */

    class material_collector                                                       //
      : public ecs::system<read_from<ecs_material>, write_to<material_resources>>, //
        public event_receiver                                                      //
    {
        public: // methods
        material_collector( resource_database& resources );
        ~material_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_material_created( ecs_material& new_material, entity& owner );
        void on_material_destroyed( const ecs_material& old_material, entity& owner );

        private: // members
        resource_database& resources;

        public: // TEMP
        vk::RenderPass temp_render_pass;
    };

    class material_reference_collector                                                                   //
      : public ecs::system<read_from<ecs_material::reference>, write_to<material_resources::reference>>, //
        public event_receiver                                                                            //
    {
        public: // methods
        material_reference_collector();
        ~material_reference_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_material_reference_created( ecs_material::reference& new_material_reference, entity& owner );
        void on_material_reference_destroyed( const ecs_material::reference& old_material_reference, entity& owner );
    };
}; // namespace rnjin::graphics::vulkan

/* -------------------------------------------------------------------------- */
/*                               Reflection Info                              */
/* -------------------------------------------------------------------------- */

namespace reflection
{
    auto_reflect_component( rnjin::graphics, vulkan::material_resources );
    auto_reflect_type( rnjin::graphics, vulkan::material_collector );
    auto_reflect_type( rnjin::graphics, vulkan::material_reference_collector );
} // namespace reflection