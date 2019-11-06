/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "entity.hpp"

#include "reflection.hpp"

namespace rnjin::ecs
{
    // Wrappers for storing references to a component type, instead of individual components
    // note: used by entity to notify the component type when it is destroyed
    class component_type_handle_base
    {
        public:
        virtual void on_entity_destroyed( entity& destroyed_entity ) const is_abstract;
    };

    template <typename T>
    class component_type_handle : public component_type_handle_base
    {
        public:
        void on_entity_destroyed( entity& destroyed_entity ) const override
        {
            // note: this check is needed, as systems can set up event handlers
            //       that leave the entity's list of owned component types in an
            //       invalid state, thus resulting in invalid removals
            if ( component<T>::is_owned_by( destroyed_entity ) )
            {
                component<T>::remove_from( destroyed_entity );
            }
        }
    };

    template <typename T>
    class component
    {
        private: // methods
        void set_owner( const entity& owner )
        {
            owner_pointer = &owner;
        }

        public: // accessors
        const entity& get_owner() const
        {
            const static entity invalid_owner{};
            check_error_condition( return invalid_owner, ecs_log_errors, owner_pointer == nullptr, "Component (\1) doesn't have a valid owner pointer", reflect<T>::get_type_name() );

            return *owner_pointer;
        }

        private: // members
        const entity* owner_pointer;

        public: // helper structures
        class owned_component
        {
            public: // methods
            owned_component( const entity::id& owner_id, const T& component_data ) : pass_member( owner_id ), pass_member( component_data ) {}
            ~owned_component() {}

            public: // members, accessors
            T component_data;
            let get_owner_id get_value( owner_id );

            private: // members
            entity::id owner_id;
        };

        protected: // static methods (accessible to friend class entity)
        friend class entity;
        friend class component_type_handle<T>;

        static let* get_type_handle_pointer()
        {
            static component_type_handle<T> handle;
            return &handle;
        }

        // Create a new component, adding it to the components global listing of all instances
        // note: constructs and copies the component into `components` since constructing in-place
        //       isn't possible with the intermediate `owned_component` struct
        // note: called by entity.add, shouldn't be called from elsewhere, since entities also manage
        //       their owned component types
        template <typename... arg_types>
        static void add_to( entity& owner, arg_types... args )
        {
            let owner_id = owner.get_id();

            let_mutable component_data = T( args... );
            component_data.set_owner( owner );

            ecs_log_verbose.print( "Add component '\2' to entity (\1)", owner_id, reflect<T>::get_type_name() );
            check_error_condition( return, ecs_log_errors, owners.count( owner_id ) > 0, "Can't add multiple instances of the same component '\2' to an entity (\1)", owner_id, reflect<T>::get_type_name() );

            // No components have been registered yet
            if ( components.empty() )
            {
                components.push_back( owned_component( owner_id, component_data ) );
                // Potentially notify others that a component of type T has been added to an entity (after it is added)
                component<T>::events.added().send( components.back().component_data, owner );
            }
            // New component should go at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            else if ( owner_id > components.back().get_owner_id() )
            {
                components.push_back( owned_component( owner_id, component_data ) );
                // Potentially notify others that a component of type T has been added to an entity (after it is added)
                component<T>::events.added().send( components.back().component_data, owner );
            }
            // New component goes somewhere in the list, perform a
            // binary search to get the appropriate location to insert
            else
            {
                // such that the `components` list remains sorted by owner IDs
                uint start = 0;
                uint end   = components.size();
                while ( start != end )
                {
                    let middle    = start + ( end - start ) / 2;
                    let middle_id = components.at( middle ).get_owner_id();
                    if ( owner_id < middle_id )
                    {
                        // continue search in left side
                        end = middle;
                    }
                    else if ( owner_id > middle_id )
                    {
                        // continue search in right side
                        start = middle + 1;
                    }
                    else
                    {
                        // ID is equal to some other id (ie the same component is added to an entity twice) this isn't allowed, but continue and notify the user
                        // note: this case should already have been covered by an earlier check of the owners set
                        start = middle;

                        let owner_id_already_exists = true;

                        check_error_condition( pass, ecs_log_errors, owner_id_already_exists == true, "Component '\2' already associated with an entity (\1)", owner_id, reflect<T>::get_type_name() );
                        break;
                    }
                }

                components.insert( components.begin() + start, owned_component( owner_id, component_data ) );
                // Potentially notify others that a component of type T has been added to an entity (after it is added)
                component<T>::events.added().send( components.at( start ).component_data, owner );
            }

            owners.insert( owner_id );
        }

        // Check if this component type already has an entry associated with the given entity, if not, add one, otherwise just pass through
        // note: called by entity.add, shouldn't be called from elsewhere, since entities also manage
        //       their owned component types
        template <typename... arg_types>
        static void add_unique( entity& owner, arg_types... args )
        {
            let owner_id = owner.get_id();
            if ( not is_owned_by( owner ) )
            {
                add_to( owner, args... );
            }
        }

        // Destroy the component associated with the provided entity
        // note: currently always modifies the global listing of all instances, in the future
        //       this should probably invalidate unowned components and reuse them for subsequent add_to calls
        // note: called by entity.add, shouldn't be called from elsewhere, since entities also manage
        //       their owned component types
        static void remove_from( entity& owner )
        {
            let owner_id = owner.get_id();

            ecs_log_verbose.print( "Remove component '\2' from entity (\1)", owner_id, reflect<T>::get_type_name() );

            // Since event handlers for components being removed can request other components to be removed, this
            // could be called on an entity that doesn't own this component, as it has already been removed by the entity's destructor.
            // ex. System removes B when A is removed:
            //     remove B in destructor -> remove A in destructor -> system tries to remove B again
            if ( owner.is_being_destroyed() and owners.count( owner_id ) == 0 )
            {
                ecs_log_verbose.print( "Component type '\2' has already been removed from destroyed entity (\1)", owner_id, reflect<T>::get_type_name() );
                return;
            }

            // Check if no components have been registered yet
            check_error_condition( return, ecs_log_errors, components.empty(), "Component type '\2' is not attached to any entities, can't remove (\1)", owner_id, reflect<T>::get_type_name() );
            // Check that the entity is actually an owner
            check_error_condition( return, ecs_log_errors, owners.count( owner_id ) == 0, "Can't remove component '\2' from an entity it's not attached to (\1)", owner_id, reflect<T>::get_type_name() );

            // Remove the entity from the set of owners
            owners.erase( owner_id );

            // Associated component is at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            if ( owner_id == components.back().get_owner_id() )
            {
                // Potentially notify others that a component of type T has been removed from an entity (before it is destroyed)
                events.removed().send( components.back().component_data, owner );
                components.pop_back();
            }
            // New component is somewhere in the list, perform a
            // binary search to get the appropriate location to remove
            // such that the `components` list remains sorted by owner IDs
            else
            {
                uint start = 0;
                uint end   = components.size();
                while ( start != end )
                {
                    let middle    = start + ( end - start ) / 2;
                    let middle_id = components.at( middle ).get_owner_id();
                    if ( owner_id < middle_id )
                    {
                        // continue search in left side
                        end = middle;
                    }
                    else if ( owner_id > middle_id )
                    {
                        // continue search in right side
                        start = middle + 1;
                    }
                    else
                    {
                        start = middle;
                        break;
                    }
                }

                if ( owner_id == components.at( start ).get_owner_id() )
                {
                    // Potentially notify others that a component of type T has been removed from an entity (before it is destroyed)
                    events.removed().send( components.at( start ).component_data, owner );
                    components.erase( components.begin() + start );
                }
                else
                {
                    let owner_id_not_found = true;
                    check_error_condition( pass, ecs_log_errors, owner_id_not_found == true, "Component '\2' not associated with entity (\1)", owner_id, reflect<T>::get_type_name() );
                }
            }
        }

        static T* owned_by( const entity& owner )
        {
            let owner_id = owner.get_id();

            // check_error_condition( return nullptr, ecs_log_errors, owners.count( owner_id ) == 0, "No components are attached to entity (\1)", owner_id );
            if ( owners.count( owner_id ) == 0 )
            {
                return nullptr;
            }

            // Associated component is at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            if ( owner_id == components.back().get_owner_id() )
            {
                return &( components.back().component_data );
            }
            // Associated component is somewhere in the list, perform a
            // binary search to get the appropriate location to return
            else
            {
                uint start = 0;
                uint end   = components.size();
                while ( start != end )
                {
                    let middle    = start + ( end - start ) / 2;
                    let middle_id = components.at( middle ).get_owner_id();
                    if ( owner_id < middle_id )
                    {
                        // continue search in left side
                        end = middle;
                    }
                    else if ( owner_id > middle_id )
                    {
                        // continue search in right side
                        start = middle + 1;
                    }
                    else
                    {
                        start = middle;
                        break;
                    }
                }

                if ( owner_id == components.at( start ).get_owner_id() )
                {
                    return &( components.at( start ).component_data );
                }
            }

            return nullptr;
        }

        static bool is_owned_by( const entity& owner )
        {
            return owners.count( owner.get_id() ) > 0;
        }

        public: // static methods (used by systems)
        // Get an iterator over all components associated with entities using constant references
        static const_iterator<owned_component> get_const_iterator()
        {
            return const_iterator<owned_component>( components );
        }

        // Get an iterator over all components associated with entities using mutable references
        static mutable_iterator<owned_component> get_mutable_iterator()
        {
            return mutable_iterator<owned_component>( components );
        }


        public: // static members
        // Events for easily performing actions on component add/remove
        static group component_events
        {
            public: // accessors
            let_mutable& added get_mutable_value( component_added_event );
            let_mutable& removed get_mutable_value( component_removed_event );

            private: // members
            event<T&, entity&> component_added_event{ "component added" };
            event<const T&, entity&> component_removed_event{ "component removed" };
        }
        events;

        protected: // static members
        // A contiguous array storing component data for efficient iteration
        static list<owned_component> components;
        // A set of owner id values, used to efficiently check if a given entity is an owner
        static set<entity::id> owners;
    };

    // Static member definitions
    template <typename T>
    typename component<T>::component_events component<T>::events;
    template <typename T>
    list<typename component<T>::owned_component> component<T>::components;
    template <typename T>
    set<typename entity::id> component<T>::owners;

#define component_class( name ) class name : public rnjin::ecs::component<name>
} // namespace rnjin::ecs