/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "component.hpp"

namespace rnjin::ecs
{
    class entity
    {
        public: // methods
        entity();
        ~entity();

        // template <typename component_type>
        // const component_type* get_component() const
        // {
        //     return component_type::owned_by( *this );
        // }

        // template <typename data_type>
        // const data_type* get_component_data() const
        // {
        //     return component<data_type>::owned_by( *this ).get_data();
        // }

        public: // accessors
        let get_id get_value( id );

        private: // members
        uint id;
    };
} // namespace rnjin::ecs