/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "entity.hpp"

#include rnjin_module( core )
#include rnjin_module( reflection )

namespace rnjin::ecs
{
    /* -------------------------------------------------------------------------- */
    /*                              Component Helpers                             */
    /* -------------------------------------------------------------------------- */

    // Wrappers for storing references to a component type, instead of individual components
    // (essentially enables virtual static methods)
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
                // forward the call to a static method on the appropriate component type
                component<T>::remove_from( destroyed_entity );
            }
        }
    };

    // // Defined below
    // template <typename T>
    // class reference;

    /* -------------------------------------------------------------------------- */
    /*                                  Component                                 */
    /* -------------------------------------------------------------------------- */

    // Specializing the component type creates a new global manager for components of that type.
    // General usage (for ease of use) would be `class MyComponent : public component<MyComponent> {...};`

    // note: alternatively, use the component_class macro `component_class( MyComponent ) {...};`

    // note<1>: it would probably be better to not have a pointer to the owner on the component itself, but is currently
    //          available so systems can get easily get the owner of a component they're operating on
    template <typename T>
    class component
    {
        public: // methods
        // no_copy( component );
        // component() : owner_pointer( nullptr ) {}

        private: // methods
        // note: see <1>
        void set_owner( const entity& owner )
        {
            owner_pointer = &owner;
        }

        public: // accessors
        // note: see <1>
        const entity& get_owner() const
        {
            const static entity invalid_owner{};
            check_error_condition( return invalid_owner, ecs_log_errors, owner_pointer == nullptr, "Component (\1) doesn't have a valid owner pointer", reflection::get_type_name<T>() );

            return *owner_pointer;
        }

        private: // members
        // note: see <1>
        const entity* owner_pointer;

        public: // helper structures
        class owned_component
        {
            public: // methods
            // no_copy( owned_component );
            // owned_component() : owner_id( entity::id::invalid() ) {}

            template <typename... arg_types>
            owned_component( const entity& owner, arg_types... args )
              : owner_id( owner.get_id() ), //
                component_data( args... )   //
            {
                component_data.set_owner( owner );
            }
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
        // friend class reference<T>;
        // friend class reference<T>;

        static let* get_type_handle_pointer()
        {
            static component_type_handle<T> handle;
            return &handle;
        }

        // Create a new component, adding it to the components global listing of all instances
        // note: really struggles handling references as constructor arguments (to entities, etc.)
        //       so for now we just always use pointers and hope for the best :(
        // note: total complexity is
        //       O( log component_count )                   to find insert position
        //     + O( component_count )                       for list reallocation if needed
        //     + O( added_event_receiver_count )            to notify systems, etc. that a component has been added
        //     + O( reference_count * log component_count ) to update potentially update pointers in all references
        template <typename... arg_types>
        static void add_to( entity& owner, arg_types... args )
        {
            let owner_id = owner.get_id();

            // let_mutable component_data = T( args... );
            // component_data.set_owner( owner );

            ecs_log_verbose.print( "Add component '\2' to entity (\1)", owner_id, reflection::get_type_name<T>() );
            check_error_condition( return, ecs_log_errors, owners.count( owner_id ) > 0, "Can't add multiple instances of the same component '\2' to an entity (\1)", owner_id, reflection::get_type_name<T>() );

            // Keep track of the newly added component so we can notify others that it was added
            T* new_component;

            // Keep track of where a component was inserted to so we can update reference indices if needed
            bool components_shifted = false;
            usize insert_index      = 0;

            // No components have been registered yet
            if ( components.empty() )
            {
                // components.push_back( owned_component( owner_id, component_data ) );
                components.emplace_back( owner, args... );
                new_component = &components.back().component_data;
            }
            // New component should go at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            else if ( owner_id > components.back().get_owner_id() )
            {
                // components.push_back( owned_component( owner_id, component_data ) );
                components.emplace_back( owner, args... );
                new_component = &components.back().component_data;
            }
            // New component goes somewhere in the list, perform a
            // binary search to get the appropriate location to insert
            else
            {
                // such that the `components` list remains sorted by owner IDs
                usize start = 0;
                usize end   = components.size();
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
                        check_error_condition( pass, ecs_log_errors, owner_id_already_exists == true, "Component '\2' already associated with an entity (\1)", owner_id, reflection::get_type_name<T>() );
                        break;
                    }
                }

                components.emplace( components.begin() + start, owner, args... );
                new_component = &components.at( start ).component_data;

                components_shifted = true;
                insert_index       = start;
            }

            // Make sure the component was actually added
            check_error_condition( return, ecs_log_errors, new_component == nullptr, "Failed to create new component '\2' for entity (\1)", owner_id, reflection::get_type_name<T>() );

            // Track the new owner
            owners.insert( owner_id );

            // If we insert a new component in the middle of the list, we need to notify references to update their indices
            if ( components_shifted )
            {
                component<T>::reallocating_component_added_event.send( insert_index );
            }

            // Potentially notify others that a component of type T has been added to an entity
            // note: happens after reallocation event to preserve correct references
            component<T>::events.added().send( *new_component, owner );
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
                ecs_log_verbose.print( "<\1>::add_unique(...) adding component to entity (\2)", reflection::get_type_name<T>(), owner_id );
                add_to( owner, args... );
            }
            else
            {
                ecs_log_verbose.print( "<\1>::add_unique(...) already owned by entity (\2)", reflection::get_type_name<T>(), owner_id );
            }
        }

        // Destroy the component associated with the provided entity
        // note: currently always modifies the global listing of all instances, in the future
        //       this should probably invalidate unowned components and reuse them for subsequent add_to calls
        // note: total complexity is
        //       O( log component_count )                   to find delete position
        //     + O( component_count )                       for list reallocation if needed
        //     + O( added_event_receiver_count )            to notify systems, etc. that a component has been removed
        //     + O( reference_count * log component_count ) to update potentially update pointers in all references
        static void remove_from( entity& owner )
        {
            let owner_id = owner.get_id();

            ecs_log_verbose.print( "Remove component '\2' from entity (\1)", owner_id, reflection::get_type_name<T>() );

            // Since event handlers for components being removed can request other components to be removed, this
            // could be called on an entity that doesn't own this component, as it has already been removed by the entity's destructor.
            // ex. System removes B when A is removed:
            //     remove B in destructor -> remove A in destructor -> system tries to remove B again
            if ( owner.is_being_destroyed() and owners.count( owner_id ) == 0 )
            {
                ecs_log_verbose.print( "Component type '\2' has already been removed from destroyed entity (\1)", owner_id, reflection::get_type_name<T>() );
                return;
            }

            // Check if no components have been registered yet
            check_error_condition( return, ecs_log_errors, components.empty(), "Component type '\2' is not attached to any entities, can't remove (\1)", owner_id, reflection::get_type_name<T>() );
            // Check that the entity is actually an owner
            check_error_condition( return, ecs_log_errors, owners.count( owner_id ) == 0, "Can't remove component '\2' from an entity it's not attached to (\1)", owner_id, reflection::get_type_name<T>() );

            // Remove the entity from the set of owners
            owners.erase( owner_id );

            // Keep track of where a component was removed from so we can update reference indices if needed
            bool components_shifted = false;
            usize removal_index     = 0;

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
                usize start = 0;
                usize end   = components.size();
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

                    components_shifted = true;
                    removal_index      = start;
                }
                else
                {
                    let owner_id_not_found = true;
                    check_error_condition( pass, ecs_log_errors, owner_id_not_found == true, "Component '\2' not associated with entity (\1)", owner_id, reflection::get_type_name<T>() );
                }
            }

            // If we remove a component in the middle of the list, we need to notify references to update their indices
            if ( components_shifted )
            {
                component<T>::reallocating_component_removed_event.send( removal_index );
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

        protected: // static accessors
        static usize index_owned_by( const entity& owner )
        {
            static let invalid_index = ~0;

            let owner_id = owner.get_id();

            // Check that the entity is actually an owner
            check_error_condition( return invalid_index, ecs_log_errors, owners.count( owner_id ) == 0, "Can't get index of component '\2' from an entity it's not attached to (\1)", owner_id,
                                          reflection::get_type_name<T>() );

            // Associated component is at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            if ( owner_id == components.back().get_owner_id() )
            {
                return components.size() - 1;
            }
            // New component is somewhere in the list, perform a
            // binary search to get the appropriate index
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
                    return start;
                }
                else
                {
                    let owner_id_not_found = true;
                    check_error_condition( pass, ecs_log_errors, owner_id_not_found == true, "Component '\2' not associated with entity (\1)", owner_id, reflection::get_type_name<T>() );
                    return invalid_index;
                }
            }
        }

        protected: // static members
        // A contiguous array storing component data for efficient iteration
        static list<owned_component> components;
        // A set of owner id values, used to efficiently check if a given entity is an owner
        static set<entity::id> owners;
        // Events used specifically be component references
        static event<usize> reallocating_component_added_event;
        static event<usize> reallocating_component_removed_event;

        /* -------------------------------------------------------------------------- */
        /*                            Component References                            */
        /* -------------------------------------------------------------------------- */
        public:
        class reference : public component<reference>
        {
            public: // methods
            reference( const T* target_pointer )
            {
                set_target( target_pointer );

                if ( not reference::handlers_initialized )
                {
                    reference::initialize_handlers();
                }
            }
            reference( const entity* target_owner_pointer )
            {
                set_target_from_owner( target_owner_pointer );

                if ( not reference::handlers_initialized )
                {
                    reference::initialize_handlers();
                }
            }
            reference( const reference& other ) : target_index( other.target_index ) {}
            ~reference() {}

            void set_target( const T* target_pointer )
            {
                let& target_owner = target_pointer->get_owner();
                target_index      = T::index_owned_by( target_owner );
            }
            void set_target_from_owner( const entity* target_owner_pointer )
            {
                target_index = T::index_owned_by( *target_owner_pointer );
            }

            public: // accessors
            const T* get_pointer() const
            {
                return &( component<T>::components[target_index].component_data );
            }
            const entity& get_referenced_owner() const
            {
                return get_pointer()->get_owner();
            }

            inline let is_valid get_value( target_index != invalid_index );

            private: // members
            usize target_index;

            private: // static methods
            static void on_component_added( usize insert_index )
            {
                ecs_log_verbose.print( "Update reference indices for '\1' (on add)", reflection::get_type_name<T>() );

                for ( nonconst auto& c : reference::components )
                {
                    // All indices after the insertion position need to be incremented
                    if ( c.component_data.target_index >= insert_index )
                    {
                        c.component_data.target_index += 1;
                    }
                }
            }

            // note: this will only be called for reallocating removals, so removing the last component in the list will not warn about invalidating active references
            //       we should probably have dedicated events for those cases in case they need to be handled for warning / etc.
            static void on_component_removed( usize remove_index )
            {
                ecs_log_verbose.print( "Update reference indices for '\1' (on remove)", reflection::get_type_name<T>() );

                for ( nonconst auto& c : components )
                {
                    let invalidating_active_reference = c.component_data.target_index == remove_index;

                    // The component at an index currently being referenced is being removed, which is bad
                    check_error_condition( continue, ecs_log_errors, invalidating_active_reference == true, "Destroying component '\1' with active references (index=\2)", reflection::get_type_name<T>(), remove_index );

                    // All indices after the deletion position need to be decremented
                    if ( c.component_data.target_index > remove_index )
                    {
                        c.component_data.target_index -= 1;
                    }
                }
            }

            // Set up handlers for components being added / removed
            // note: this is called on first instantiation of a reference type,
            //       since sequence of memory allocation isn't handled well yet
            static void initialize_handlers()
            {
                ecs_log_verbose.print( "Initialize reference handlers for '\1'", reflection::get_type_name<T>() );

                component_added_handler.set( component<T>::reallocating_component_added_event, on_component_added );
                component_removed_handler.set( component<T>::reallocating_component_removed_event, on_component_removed );
            }

            private: // static members
            static bool handlers_initialized;
            static static_event_handler<usize> component_added_handler;
            static static_event_handler<usize> component_removed_handler;

            public: // constants
            static const usize invalid_index = ~0;
        };
    };

    // static member definitions
    // clang-format off
    template <typename T> typename component<T>::component_events component<T>::events;
    template <typename T> list<typename component<T>::owned_component> component<T>::components;
    template <typename T> set<typename entity::id> component<T>::owners;
    template <typename T> event<usize> component<T>::reallocating_component_added_event { "reallocating component added" };
    template <typename T> event<usize> component<T>::reallocating_component_removed_event { "reallocating component removed" };

    template <typename T> bool component<T>::reference::handlers_initialized = false;
    template <typename T> static_event_handler<usize> component<T>::reference::component_added_handler;
    template <typename T> static_event_handler<usize> component<T>::reference::component_removed_handler;
    // clang-format on 

#define component_class( name ) class name : public rnjin::ecs::component<name>
} // namespace rnjin::ecs