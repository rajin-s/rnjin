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
            check_error_condition( return invalid_owner, ecs_log_errors, owner_pointer == nullptr, "Component (\1) doesn't have a valid owner pointer", reflect<T>::get_type_name() );

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

        // Since components are stored in a list (c++ vector) that is updated when adding/removing a component from an entity,
        // it is impossible to store direct pointers (or references) without the risk of them becoming invalidated. The
        // reference member type handles this by lazily setting up pointers based on a component's owner: any time the location
        // of that component changes or it is created/destroyed, the internal pointer will be updated.
        // note: this could also be used to do reference counting inside the component type, which might be useful???
        //       Maybe have a special flag that subtypes can set to remove components that lose all references?
        class reference
        {
            public: // methods
            // Create a reference based on a particular owner
            reference( const entity& owner )
              : owner_pointer( &owner ),                                                                                       //
                component_reallocated_handler( component::events.reallocated(), this, &reference::on_components_reallocated ), //
                component_added_handler( component::events.added(), this, &reference::on_component_added ),                    //
                component_removed_handler( component::events.removed(), this, &reference::on_component_removed )               //
            {
                component_pointer = component::owned_by( owner );
            }
            // Copy a reference
            reference( const reference& other )
              : owner_pointer( other.owner_pointer ),                                                                          //
                component_reallocated_handler( component::events.reallocated(), this, &reference::on_components_reallocated ), //
                component_added_handler( component::events.added(), this, &reference::on_component_added ),                    //
                component_removed_handler( component::events.removed(), this, &reference::on_component_removed )               //
            {}
            ~reference() {}

            // Allow reference->... syntax to access the internal component pointer
            // note: is const by default for safety -- get_mutable_pointer can be used for mutable access
            inline const T* operator->()
            {
                return component_pointer;
            }

            private: // methods
            // When the corresponding component list is changed, some or all elements might be moved in memory, resulting in
            // invalid or incorrect pointers. As such, component references need to update their internal pointer any time this happens.
            void on_components_reallocated()
            {
                // We don't know how the component list has been changed, so search for a component owned by the appropriate entity
                // note: this is a O(log N) search for every reference -- it may be possible to update all references with a linear
                //       pass through the component list -- complexity O( log (component_count) * reference_count ) vs. O( component_count )
                ecs_log_verbose.print( "component_reference<\1>::on_components_reallocated", reflect<T>::get_type_name() );
                component_pointer = component::owned_by( *owner_pointer );
            }
            void on_component_added( T& new_component, entity& new_owner )
            {
                // A component has been added to some entity, so we check if it's the owner and validate the internal pointer as appropriate
                // note: this will be called on all references for every component that is added -- it would probably be better to move have
                //       the source event live in the entity, since that is fixed when a reference is created
                ecs_log_verbose.print( "component_reference<\1>::on_component_added", reflect<T>::get_type_name() );
                if ( owner_pointer->get_id() == new_owner.get_id() )
                {
                    component_pointer = &new_component;
                }
            }
            void on_component_removed( const T& old_component, entity& old_owner )
            {
                // A component has been removed from some entity, so we check if it's the owner and invalidate the internal pointer as appropriate
                ecs_log_verbose.print( "component_reference<\1>::on_component_removed", reflect<T>::get_type_name() );
                if ( owner_pointer->get_id() == old_owner.get_id() )
                {
                    component_pointer = nullptr;
                }
            }

            public: // accessors
            // A reference can be invalid if the associated owner doesn't have the component (it has not yet been added or has since been removed)
            let is_valid get_value( component_pointer != nullptr );
            let* get_pointer get_value( component_pointer );
            let_mutable* get_mutable_pointer get_mutable_value( component_pointer );

            // Get the owner of the (potentially) referenced component
            let& get_owner get_value( *owner_pointer );

            private: // members
            // note: we have to store a pointer, as components might be un-initialized during list (c++ vector) allocation preventing use of a reference
            //       if we want to store references inside components (which is the whole point)
            const entity* owner_pointer;
            T* component_pointer;

            event_handler<reference> component_reallocated_handler;
            event_handler<reference, T&, entity&> component_added_handler;
            event_handler<reference, const T&, entity&> component_removed_handler;
        };

        protected: // static methods (accessible to friend class entity)
        friend class entity;
        friend class component_type_handle<T>;
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

            ecs_log_verbose.print( "Add component '\2' to entity (\1)", owner_id, reflect<T>::get_type_name() );
            check_error_condition( return, ecs_log_errors, owners.count( owner_id ) > 0, "Can't add multiple instances of the same component '\2' to an entity (\1)", owner_id, reflect<T>::get_type_name() );

            // Keep track of the original data container pointer so we can tell if the vector was
            // re-allocated as a result of adding a new element
            let* original_component_data_pointer = components.data();
            bool add_to_middle                   = false;

            // Keep track of the newly added component so we can notify others that it was added
            T* new_component;

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
                        start         = middle;
                        add_to_middle = true;

                        let owner_id_already_exists = true;
                        check_error_condition( pass, ecs_log_errors, owner_id_already_exists == true, "Component '\2' already associated with an entity (\1)", owner_id, reflect<T>::get_type_name() );
                        break;
                    }
                }

                // components.insert( components.begin() + start, owned_component( owner_id, component_data ) );
                components.emplace( components.begin() + start, owner, args... );
                new_component = &components.at( start ).component_data;
            }

            // Make sure the component was actually added
            check_error_condition( return, ecs_log_errors, new_component == nullptr, "Failed to create new component '\2' for entity (\1)", owner_id, reflect<T>::get_type_name() );

            // Track the new owner
            owners.insert( owner_id );

            // Check if the data container pointer has moved, potentially notifying others that need to react accordingly
            // note: used by component_reference to find the correct component pointer for a given entity
            let* new_component_data_pointer = components.data();
            if ( original_component_data_pointer != new_component_data_pointer || add_to_middle )
            {
                ecs_log_verbose.print_additional( "triggered reallocation" );
                component<T>::events.reallocated().send();
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
                ecs_log_verbose.print( "<\1>::add_unique(...) adding component to entity (\2)", reflect<T>::get_type_name(), owner_id );
                add_to( owner, args... );
            }
            else
            {
                ecs_log_verbose.print( "<\1>::add_unique(...) already owned by entity (\2)", reflect<T>::get_type_name(), owner_id );
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

            // Keep track of the original data container pointer so we can tell if
            // the vector was re-allocated as a result of removing an element
            let* original_component_data_pointer = components.data();
            bool removed_from_middle             = false;

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
                    removed_from_middle = true;
                }
                else
                {
                    let owner_id_not_found = true;
                    check_error_condition( pass, ecs_log_errors, owner_id_not_found == true, "Component '\2' not associated with entity (\1)", owner_id, reflect<T>::get_type_name() );
                }
            }

            // Check if the data container pointer has moved, potentially notifying others that need to react accordingly
            // note: used by component_reference to find the correct component pointer for a given entity
            let* new_component_data_pointer = components.data();
            if ( original_component_data_pointer != new_component_data_pointer || removed_from_middle )
            {
                ecs_log_verbose.print_additional( "triggered reallocation" );
                component<T>::events.reallocated().send();
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
            let_mutable& reallocated get_mutable_value( components_reallocated_event );

            private: // members
            event<T&, entity&> component_added_event{ "component added" };
            event<const T&, entity&> component_removed_event{ "component removed" };
            event<> components_reallocated_event{ "components reallocated" };
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