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

    component_class( model_resources )
    {
        public: // methods
        model_resources();
        ~model_resources();

        private: // members
        friend class model_collector;
    };

    /* -------------------------------------------------------------------------- */
    /*                                   Systems                                  */
    /* -------------------------------------------------------------------------- */

    class model_collector //
      : public ecs::system<read_from<ecs_model>, write_to<model_resources>>,
        public event_receiver //
    {
        public: // methods
        model_collector();
        ~model_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_model_created( ecs_model& new_model, entity& owner );
        void on_model_destroyed( const ecs_model& old_model, entity& owner );
    };
}; // namespace rnjin::graphics::vulkan

/* -------------------------------------------------------------------------- */
/*                               Reflection Info                              */
/* -------------------------------------------------------------------------- */

namespace reflection
{
    auto_reflect_component( rnjin::graphics, vulkan::model_resources );
    auto_reflect_type( rnjin::graphics, vulkan::model_collector );
} // namespace reflection