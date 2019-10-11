/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"

namespace rnjin::ecs
{
    template <typename T>
    class component_base
    {
        public: // methods
        template <typename... arg_types>
        static const T& create( arg_types... args )
        {
            data.emplace_back( args... );
            return data.back();
        }

        protected:
        static list<T> data;
    };

    class test_component : component_base<test_component>
    {
        public:
        test_component( float foo, int bar ) : pass_member( foo ), pass_member( bar ) {}
        ~test_component();

        float foo;
        int bar;
    };

    // class entity;

    // template <typename data_type>
    // class component
    // {
    //     public: // methods
    //     component( const entity& owner ) : pass_member( owner )
    //     {
    //         id = component::data.get_next_id();
    //         ownership.add_new_owned_component( owner, this );
    //     }
    //     ~component()
    //     {
    //         component::data.invalidate( id );
    //         ownership.remove_owned_component( owner );
    //     }

    //     public: // accessors
    //     let& get_data get_value( data.get_data( id ) );

    //     private: // members
    //     const entity& owner;
    //     uint id;

    //     public: // static methods
    //     component* owned_by( const entity& owner )
    //     {
    //         return component::ownership.get_component_owned_by( owner );
    //     }

    //     private: // static members
    //     static group
    //     {
    //         public: // methods
    //         void add_new_owned_component( const entity& owner, component* new_component )
    //         {
    //             let owner_id = owner.get_id();

    //             // TODO: check includes to allow error checking
    //             // check_error_condition( pass, ecs_log_errors, owned_components.count( owner_id ) > 0, "Adding multiple components of the same type to a single owner is not supported" );

    //             owned_components.insert( owner_id, new_component );
    //         }

    //         void remove_owned_component( const entity& owner )
    //         {
    //             let owner_id = owner.get_id();

    //             // TODO: check includes to allow error checking
    //             // check_error_condition( pass, ecs_log_errors, owned_components.count( owner_id ) > 0, "Trying to remove component without valid owner" );

    //             owned_components.erase( owner_id );
    //         }

    //         component* get_component_owned_by( const entity& owner )
    //         {
    //             if ( owned_components.count( owner.get_id() ) == 0 )
    //             {
    //                 return nullptr;
    //             }
    //             else
    //             {
    //                 return owned_components.at( owner.get_id() );
    //             }
    //         }

    //         private: // members
    //         dictionary<uint, component*> owned_components;
    //     }
    //     ownership{};

    //     static group
    //     {
    //         public: // methods
    //         // Fetch the data associated with a component id
    //         data_type& get_data( const uint id )
    //         {
    //             return entries[id].data;
    //         }

    //         // Get the index of the next available component data, which means
    //         //      validated a previously invalidated data entry
    //         //      creating a new (valid) data entry
    //         uint get_next_id()
    //         {
    //             // Record that another live component has been created
    //             // note: this could be called from the component constructor,
    //             //       currently that is the only place get_next_id is called,
    //             //       so it's effectively the same
    //             record_component_created();

    //             // Short-circuit if there aren't any invalid entries,
    //             // inserting a new element and reallocating as needed
    //             if ( next_invalid_entry == no_invalid_entry )
    //             {
    //                 uint result = entries.size();
    //                 entries.emplace_back();

    //                 return result;
    //             }
    //             else
    //             {
    //                 let result = next_invalid_entry;

    //                 // Refresh the last found invalid entry
    //                 data_entry& invalid_entry = entries[next_invalid_entry];
    //                 invalid_entry.valid       = true;
    //                 invalid_entry.data.reset();

    //                 // Scan all entries, recording the next invalid one found
    //                 bool found_invalid_entry = false;
    //                 for ( uint i : range( entries.size() ) )
    //                 {
    //                     if ( not entries[i].valid )
    //                     {
    //                         found_invalid_entry = true;
    //                         next_invalid_entry  = i;
    //                         break;
    //                     }
    //                 }

    //                 // If no other invalid elements were found, reset next_invalid_entry
    //                 if ( not found_invalid_entry )
    //                 {
    //                     next_invalid_entry = no_invalid_entry;
    //                 }

    //                 return result;
    //             }
    //         }

    //         // Mark the entry at index=id as invalid
    //         void invalidate( const uint id )
    //         {
    //             entries[id].valid = false;
    //             if ( next_invalid_entry == no_invalid_entry )
    //             {
    //                 next_invalid_entry = id;
    //             }

    //             // Record that a live component has been destroyed
    //             // note: this could be called from the component destructor,
    //             //       currently that is the only place invalidate is called,
    //             //       so it's effectively the same
    //             record_component_destroyed();
    //         }

    //         private: // methods
    //         void record_component_created()
    //         {
    //             // Increment the total number of live components
    //             live_count += 1;
    //         }
    //         void record_component_destroyed()
    //         {
    //             // Decrement the total number of live components
    //             live_count -= 1;

    //             // If there are less than zero live components, something has gone wrong
    //             // TODO: check includes to allow error logging
    //             // check_error_condition( pass, ecs_log_errors, live_count < 0, "More components have been destroyed than have been created" );

    //             // Free memory if no live components remain
    //             // note: otherwise, memory used by data would remain allocated forever
    //             // note: this can probably be more dynamic, reallocating once half of the entries
    //             //       are invalidated, etc...
    //             if ( live_count <= 0 )
    //             {
    //                 clear_entries();
    //             }
    //         }

    //         // Release memory used for all data entries
    //         // note: called when all component instances are destroyed
    //         void clear_entries()
    //         {
    //             entries.resize( 0 );
    //         }

    //         private: // members
    //         struct data_entry
    //         {
    //             data_entry() : valid( true ), data_type() {}

    //             bool valid;
    //             data_type data;
    //         };

    //         // Number of components actively using data entries
    //         int live_count = 0;

    //         const uint no_invalid_entry = ~0;
    //         uint next_invalid_entry     = no_invalid_entry;

    //         list<data_entry> entries;
    //     }
    //     data{};
    // };
} // namespace rnjin::ecs