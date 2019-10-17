/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "log.hpp"

namespace rnjin::ecs
{
    // Global ecs logs

    // Graphics logs flags used in masked log source creation
    enum class log_flag : uint
    {
        verbose   = 2,
        errors    = 3,
        entity    = 4,
        component = 5,
        system    = 6,
    };

    extern log::source& get_ecs_log();
    extern log::source::masked ecs_log_vernbose;
    extern log::source::masked ecs_log_errors;

    class entity
    {
        public: // types
        using id = unique_id<entity>;

        public: // methods
        entity();
        ~entity();

        // Add a component to this entity
        // note: actually forwards call to component::add_to, since
        //       entities don't internally store their associated components
        template <typename component_type, typename... arg_types>
        void add( arg_types... args ) const
        {
            component<component_type>::add_to( *this, args... );
        }

        // Add a component to this entity if it doesn't already exist
        // note: actually forwards call to component::add_unique, since
        //       entities don't internally store their associated components
        template <typename component_type, typename... arg_types>
        void require( arg_types... args ) const
        {
            component<component_type>::add_unique( *this, args... );
        }

        // Remove a component from this entity
        // note: actually forwards call to component::remove_from, since
        //       entities don't internally store their associated components
        template <typename component_type>
        void remove() const
        {
            component<component_type>::remove_from( *this );
        }

        // Get a component attached to this entity
        // note: actually forwards call to component::owned_by, since
        //       entities don't internally store their associated components
        template <typename component_type>
        const component_type* get() const
        {
            return component<component_type>::owned_by( *this );
        }

        public: // accessors
        let get_id get_value( entity_id );

        private: // members
        id entity_id;
    };
} // namespace rnjin::ecs