/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "ecs.hpp"
#include "visual_ecs.hpp"
#include "vulkan_resources.hpp"

namespace rnjin::graphics::vulkan
{
    class resource_collector : public ecs::system<read_from<model>, write_to<internal_resources>>, event_receiver
    {
        public: // methods
        void initialize();

        protected: // internal
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_model_added( model& new_model, const entity& owner );
        void on_model_removed( const model& old_model, const entity& owner );
    };
} // namespace rnjin::graphics::vulkan