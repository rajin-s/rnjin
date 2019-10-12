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
        // using id = unique_id<entity>;
        using id = unique_id<entity>;

        public: // methods
        entity() {}
        ~entity() {}

        public: // accessors
        let get_id get_value( entity_id );

        private: // members
        id entity_id;
    };
} // namespace rnjin::ecs