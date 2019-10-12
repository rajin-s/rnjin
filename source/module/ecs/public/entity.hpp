/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"

namespace rnjin::ecs
{
    class entity
    {
        public: // types
        using id = unique_id<entity>;

        public: // methods
        entity() {}
        ~entity() {}

        // Add a component to this entity
        // note: actually forwards call to component::add_to, since
        //       entities don't internally store their associated components
        template <typename component_type, typename... arg_types>
        void add( arg_types... args )
        {
            component_type::add_to( *this, args... );
        }

        // Remove a component from this entity
        // note: actually forwards call to component::remove_from, since
        //       entities don't internally store their associated components
        template <typename component_type>
        void remove()
        {
            component_type::remove_from( *this );
        }

        public: // accessors
        let get_id get_value( entity_id );

        private: // members
        id entity_id;
    };
} // namespace rnjin::ecs