/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "entity.hpp"
#include "component.hpp"

#include rnjin_module( core )

namespace rnjin::ecs
{
    // Wrapper type for constant references to component data
    // note: used in system template specialization (system<read_from<T>, ...>)
    template <typename component_type>
    struct read_from
    {
        read_from( const component_type& source ) : pass_member( source ) {}
        const component_type& source;
    };

    // Wrapper type for mutable references to component data
    // note: used in system template specialization (system<write_to<T>, ...>)
    template <typename component_type>
    struct write_to
    {
        write_to( nonconst component_type& destination ) : pass_member( destination ) {}
        nonconst component_type& destination;
    };

    // Base class to derive new systems from
    // note: systems are defined on the set of components they work with, tagged
    //       with read_from<T> or write_to<T> depending on how each component needs
    //       to be accessed (ex. my_system : public system<read_from<my_component>, write_to<other_component>>)
    template <typename... accessor_types>
    class system
    {
        protected: // types
        // A grouping of associated components this system operates on, accessible through access types (read_to, write_from)
        // note: components are associated if they're owned by the same entity
        class entity_components
        {
            public: // methods
            entity_components( accessor_types... accessors ) : accessors( accessors... ) {}

            // Get a constant reference to the collection member of type T
            // note: will fail type checking if the system is not defined on read_from<T>
            template <typename T>
            const T& readable()
            {
                let& accessor = std::get<get_tuple_index_by_type<read_from<T>, 0, accessor_types...>::index>( accessors );
                return accessor.source;
            }

            // Get a mutable reference to the collection member of type T
            // note: will fail type checking if the system is not defined on write_to<T>
            template <typename T>
            nonconst T& writable()
            {
                let& accessor = std::get<get_tuple_index_by_type<write_to<T>, 0, accessor_types...>::index>( accessors );
                return accessor.destination;
            }

            private: // members
            std::tuple<accessor_types...> accessors;

            private: // helpers for coercing compile-time data using the type system
            // Error case of finding an index of a tuple by type
            //      Should return an index that is out of bounds
            template <typename T_search, usize N, typename... Ts>
            struct get_tuple_index_by_type
            {
                static constexpr usize index = N;
            };
            // Terminal case of finding an index of a tuple by type
            //      value is the number of steps it took to get here
            template <typename T_search, usize N, typename... T_rest>
            struct get_tuple_index_by_type<T_search, N, T_search, T_rest...>
            {
                static constexpr usize index = N;
            };
            // Non-terminal case of finding an index of a tuple by type
            //      Continue search in rest of types
            template <typename T_search, usize N, typename T_first, typename... T_rest>
            struct get_tuple_index_by_type<T_search, N, T_first, T_rest...>
            {
                static constexpr usize index = get_tuple_index_by_type<T_search, N + 1, T_rest...>::index;
            };
        };

        protected: // virtual methods
        virtual void define() pure_virtual;

        virtual void update( entity_components& components ) pure_virtual;
        
        virtual void before_update() {}
        virtual void after_update() {}

        protected: // methods
        template <typename system_type>
        void depends_on()
        {
            // TODO: Handle dependency graph generation between systems that rely on the output of other systems
        }

        public:
        // Call `update` method on all groupings of entity-owned components that this system operates on
        void update_all()
        {
            before_update();

            entity_iterator<accessor_types...> all;
            while ( all.has_next() )
            {
                update( all.get_next() );
            }

            after_update();
        }

        private: // helpers
        // Terminal case (no accessors left)
        // note: could still have invalid template parameters (not read_from or write_to), but that
        //       should already give an error elsewhere
        template <typename... Ts>
        struct entity_iterator
        {
            // If we've gotten here, all parent iterators have returned true
            inline bool has( entity::id target )
            {
                return true;
            }

            // All parent iterators have constructed parameters, so return the final structure
            template <typename... Ts>
            inline entity_components get_next_append( Ts... previous )
            {
                return entity_components( previous... );
            }
        };

        template <typename T_first, typename... T_rest>
        struct entity_iterator<read_from<T_first>, T_rest...>
        {
            entity_iterator() : component_iterator( T_first::get_const_iterator() ) {}
            const_iterator<typename component<T_first>::owned_component> component_iterator;

            // Move all iterators along until they are all at the same ID.
            // returns true if such an entry exists, false otherwise
            // note: meant to only be called on the 'top-level' entity_iterator
            inline bool has_next()
            {
                // Keep going in this iterator until it's invalid
                // TODO: short circuit if any other iterators become invalid
                while ( component_iterator.is_valid() )
                {
                    // Get the current entry for this component access iterator
                    let next_id = ( *component_iterator ).get_owner_id();

                    // Advance other iterators as needed until a match is found or we know one won't be found
                    // (other iterator points to a higher ID, or is invalid)
                    bool others_have_id = others.has( next_id );

                    if ( others_have_id )
                    {
                        // All other iterators have the same ID, so leave state as-is and return true
                        return true;
                    }
                    else
                    {
                        // Other iterators don't have this ID, so advance this one and try the next entry
                        // TODO: advance this iterator to the max position of others' iterators, since nothing between will be shared
                        component_iterator.advance();
                    }
                }

                // This iterator is invalid, so no other entries can exist as shared
                return false;
            }

            // Move all iterators forward until they match the target ID or we know they will never match
            // (points to higher ID, or is invalid). Returns true if all iterators were able to align, false otherwise
            // note: meant to only be called on 'child' associated_iterators, called from has_next of the 'top-level'
            inline bool has( entity::id target )
            {
                while ( component_iterator.is_valid() )
                {
                    let next_id = ( *component_iterator ).get_owner_id();

                    if ( next_id == target )
                    {
                        // This iterator has aligned, so leave state as-is and check the next
                        return others.has( target );
                    }
                    else if ( next_id > target )
                    {
                        // This iterator has passed the target, so leave state as-is and report that no alignment is possible
                        return false;
                    }
                    else
                    {
                        // The target could still be found, so advance this iterator and repeat
                        component_iterator.advance();
                    }
                }

                // This iterator is invalid, so no alignment is possible
                return false;
            }

            // Go through each iterator and collect all current entries into an entity_components structure
            // note: called on 'top-level' entity_iterator, calls get_next_append to aggregate references into final structure
            inline entity_components get_next()
            {
                entity_components result = others.get_next_append( read_from( ( *component_iterator ).component_data ) );
                component_iterator.advance();
                return result;
            }

            // Go through each remaining iterator and collect current entries into an entity_components structure
            // note: called on 'child' entity_iterators from get_next in the 'top-level' one
            template <typename... Ts>
            inline entity_components get_next_append( Ts... previous )
            {
                entity_components result = others.get_next_append( previous..., read_from( ( *component_iterator ).component_data ) );
                component_iterator.advance();
                return result;
            }

            private:
            entity_iterator<T_rest...> others;
        };

        // note: virtually the same as above
        // TODO: create a single base template that these can specialize, if possible
        template <typename T_first, typename... T_rest>
        struct entity_iterator<write_to<T_first>, T_rest...>
        {
            // note: these lines vary between read_from and write_to cases
            entity_iterator() : component_iterator( T_first::get_mutable_iterator() ) {}
            mutable_iterator<typename component<T_first>::owned_component> component_iterator;

            // Move all iterators along until they are all at the same ID.
            // returns true if such an entry exists, false otherwise
            // note: only called on the 'top-level' entity_iterator
            // note: this method doesn't vary between read_from and write_to cases
            inline bool has_next()
            {
                // Keep going in this iterator until it's invalid
                // TODO: short circuit if any other iterators become invalid
                while ( component_iterator.is_valid() )
                {
                    // Get the current entry for this component access iterator
                    let next_id = ( *component_iterator ).get_owner_id();

                    // Advance other iterators as needed until a match is found or we know one won't be found
                    // (other iterator points to a higher ID, or is invalid)
                    bool others_have_id = others.has( next_id );

                    if ( others_have_id )
                    {
                        // All other iterators have the same ID, so leave state as-is and return true
                        return true;
                    }
                    else
                    {
                        // Other iterators don't have this ID, so advance this one and try the next entry
                        // TODO: advance this iterator to the max position of others' iterators, since nothing between will be shared
                        component_iterator.advance();
                    }
                }

                // This iterator is invalid, so no other entries can exist as shared
                return false;
            }

            // Move all iterators forward until they match the target ID or we know they will never match
            // (points to higher ID, or is invalid). Returns true if all iterators were able to align, false otherwise
            // note: only called on 'child' entity_iterators, called from has_next of the 'top-level'
            // note: this method doesn't vary between read_from and write_to cases
            inline bool has( entity::id target )
            {
                while ( component_iterator.is_valid() )
                {
                    let next_id = ( *component_iterator ).get_owner_id();

                    if ( next_id == target )
                    {
                        // This iterator has aligned, so leave state as-is and check the next
                        return others.has( target );
                    }
                    else if ( next_id > target )
                    {
                        // This iterator has passed the target, so leave state as-is and report that no alignment is possible
                        return false;
                    }
                    else
                    {
                        // The target could still be found, so advance this iterator and repeat
                        component_iterator.advance();
                    }
                }

                // This iterator is invalid, so no alignment is possible
                return false;
            }

            // Go through each iterator and collect all current entries into an entity_components structure
            // note: called on 'top-level' entity_iterator, calls get_next_append to aggregate references into final structure
            // note: this method varies between read_from and write_to cases
            inline entity_components get_next()
            {
                entity_components result = others.get_next_append( write_to( ( *component_iterator ).component_data ) );
                component_iterator.advance();
                return result;
            }

            // Go through each remaining iterator and collect current entries into an entity_components structure
            // note: called on 'child' entity_iterators from get_next in the 'top-level' one
            // note: this method varies between read_from and write_to cases
            template <typename... Ts>
            inline entity_components get_next_append( Ts... previous )
            {
                entity_components result = others.get_next_append( previous..., write_to( ( *component_iterator ).component_data ) );
                component_iterator.advance();
                return result;
            }

            private:
            entity_iterator<T_rest...> others;
        };
    };
} // namespace rnjin::ecs