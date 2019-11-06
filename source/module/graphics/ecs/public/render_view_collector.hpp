/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "graphics/public/render_view.hpp"
#include "model.hpp"
#include rnjin_module( ecs )

namespace rnjin::graphics
{
    class render_view_collector : public ecs::system<read_from<model>>
    {
        public: // methods
        render_view_collector( render_view& target_view ) : pass_member( target_view ) {}

        void define() {}
        void initialize() {}

        protected: // inherited
        void update( entity_components& components )
        {
            let& model_data = components.readable<model>();
            target_view.add_item( model_data.get_mesh(), model_data.get_material() );
        }

        private: // members
        render_view& target_view;
    };
} // namespace rnjin::graphics