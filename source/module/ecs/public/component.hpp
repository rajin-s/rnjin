/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "entity.hpp"

namespace rnjin::ecs
{
    template <typename T>
    class component
    {
        public: // structures
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

        public: // static methods
        // Create a new component, adding it to the components global listing of all instances
        // note: constructs and copies the component into `components` since constructing in-place
        //       isn't possible with the intermediate `owned_component` struct
        template <typename... arg_types>
        static T& add_to( const entity& owner, arg_types... args )
        {
            let owner_id       = owner.get_id();
            let component_data = T( args... );

            // No components have been registered yet
            if ( components.empty() )
            {
                components.push_back( owned_component( owner_id, component_data ) );
            }
            // New component should go at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            else if ( owner_id > components.back().get_owner_id() )
            {
                components.push_back( owned_component( owner_id, component_data ) );
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
                        // ID is equal to some other id (ie the same component is added to an entity twice)
                        // this isn't allowed, but continue and notify the user
                        start = middle;

                        let owner_id_already_exists = true;
                        // TODO: fix includes to get error checking working
                        // check_error_condition( pass, ecs_log_errors, owner_id_already_exists == true, "Can't add multiple instances of the same component to entity (\1)", owner_id );

                        break;
                    }
                }

                components.insert( components.begin() + start, owned_component( owner_id, component_data ) );
            }

            return components.back().component_data;
        }

        static void remove_from( const entity& owner )
        {
            let owner_id = owner.get_id();

            // No components have been registered yet
            if ( components.empty() )
            {
                // TODO: emit warning
            }
            // New component is at end of list
            // note: would be handled by binary search below, but this is a common case that can be easily optimized
            else if ( owner_id == components.back().get_owner_id() )
            {
                components.pop_back();
            }
            // New component is somewhere in the list, perform a
            // binary search to get the appropriate location to remove
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
                        start = middle;
                        break;
                    }
                }

                if ( owner_id == components.at( start ).get_owner_id() )
                {
                    components.erase( components.begin() + start );
                }
                else
                {
                    // TODO: emit warning
                }
            }
        }
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

        protected: // static members
        // A contiguous array storing component data for efficient iteration
        static list<owned_component> components;
    };

    template <typename T>
    list<typename component<T>::owned_component> component<T>::components{};

#define component_class( name ) class name : public component<name>
} // namespace rnjin::ecs