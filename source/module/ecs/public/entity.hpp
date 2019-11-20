/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include rnjin_module( core )
#include rnjin_module( log )

namespace rnjin::ecs
{
    // Global ECS logs

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
    extern log::source::masked ecs_log_verbose;
    extern log::source::masked ecs_log_errors;

    class component_type_handle_base;

    class entity
    {
        no_copy( entity );

        public: // types
        using id = unique_id<entity>;

        public: // methods
        entity();
        ~entity();

        // Add a component to this entity
        // note: actually forwards call to component::add_to, since
        //       entities don't internally store their associated components
        template <typename component_type, typename... arg_types>
        void add( arg_types... args )
        {
            component<component_type>::add_to( *this, args... );
            add_component_type_handle( component<component_type>::get_type_handle_pointer() );
        }

        // Add a component to this entity if it doesn't already exist
        // note: actually forwards call to component::add_unique, since
        //       entities don't internally store their associated components
        template <typename component_type, typename... arg_types>
        void require( arg_types... args )
        {
            component<component_type>::add_unique( *this, args... );
            add_component_type_handle( component<component_type>::get_type_handle_pointer() );
        }

        // Remove a component from this entity
        // note: actually forwards call to component::remove_from, since
        //       entities don't internally store their associated components
        template <typename component_type>
        void remove()
        {
            component<component_type>::remove_from( *this );
            remove_component_type_handle( component<component_type>::get_type_handle_pointer() );
        }

        // Get a component attached to this entity
        // note: actually forwards call to component::owned_by, since
        //       entities don't internally store their associated components
        template <typename component_type>
        const component_type* get() const
        {
            return component<component_type>::owned_by( *this );
        }

        // Get a component attached to this entity as a mutable pointer
        // note: actually forwards call to component::owned_by, since
        //       entities don't internally store their associated components
        template <typename component_type>
        component_type* get_mutable() const
        {
            return component<component_type>::owned_by( *this );
        }

        inline bool operator==( const entity& other ) const
        {
            return entity_id == other.entity_id;
        }

        private: // methods
        void add_component_type_handle( const component_type_handle_base* type_handle_pointer );
        void remove_component_type_handle( const component_type_handle_base* type_handle_pointer );

        public: // accessors
        let get_id get_value( entity_id );
        let is_being_destroyed get_value( destroying );

        private: // members
        id entity_id;
        bool destroying;
        set<const component_type_handle_base*> owned_component_types;

        // public: // static
        // static group
        // {
        //     public: // accessors
        //     // note: these events are used in initialization of other static data (component event handlers)
        //     //       so they are accessed via static function variables to ensure they are initialized properly
        //     event<const entity&>& created()
        //     {
        //         static event<const entity&> created_event( "entity created" );
        //         return created_event;
        //     };
        //     event<const entity&>& destroyed()
        //     {
        //         static event<const entity&> destroyed_event( "entity destroyed" );
        //         return destroyed_event;
        //     };
        // }
        // events;
    };
} // namespace rnjin::ecs