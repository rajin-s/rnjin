/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "ecs/public/ecs.hpp"
#include "graphics/ecs/public/visual_ecs.hpp"
#include "vulkan_renderer.hpp"
#include "vulkan_resources.hpp"

namespace rnjin::graphics::vulkan
{
    class resource_collector : public ecs::system<read_from<model>, write_to<internal_resources>>, event_receiver
    {
        public: // methods
        resource_collector( const renderer& target_renderer );
        ~resource_collector();

        void initialize( const resource_database::initialization_info resource_database_info );

        protected: // internal
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_model_added( model& new_model, entity& owner );
        void on_model_removed( const model& old_model, entity& owner );

        private: // members
        const renderer& target_renderer;
        resource_database resources;
    };
} // namespace rnjin::graphics::vulkan

/* -------------------------------------------------------------------------- */
/*                               Reflection Info                              */
/* -------------------------------------------------------------------------- */

reflection_info_for( rnjin::graphics::vulkan, resource_collector )
{
    reflect_type_name( "vulkan::resource_collector" );
};