/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "entity.hpp"
#include "component.hpp"

namespace rnjin::ecs
{
    template <typename component_type>
    struct read_from
    {
        const component_type& component;
    };

    template <typename component_type>
    struct write_to
    {
        nonconst component_type& component;
    };

    template <typename... accessor_types>
    class system
    {
        protected: // types
        class component_collection
        {
            public: // methods
            component_collection( accessor_types... accessors ) : accessors( accessors... ) {}

            // Get a constant reference to the collection member of type T
            // note: will fail type checking if the system is not defined on read_from<T>
            template <typename T>
            const T& get_read()
            {
                let& accessor = std::get<get_tuple_index_by_type<read_from<T>, 0, accessor_types...>::index>( accessors );
                return accessor.component;
            }

            // Get a mutable reference to the collection member of type T
            // note: will fail type checking if the system is not defined on write_to<T>
            template <typename T>
            nonconst T& get_write()
            {
                let& accessor = std::get<get_tuple_index_by_type<write_to<T>, 0, accessor_types...>::index>( accessors );
                return accessor.component;
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
        virtual void define() is_abstract;
        virtual void initialize() is_abstract;
        virtual void update_collection( component_collection& collection ) is_abstract;

        protected: // methods
        template <typename system_type>
        void depends_on(){};
    };

    class test_system : system<read_from<test_component>>
    {
        void define() {}
        void initialize() {}
        void update_collection( component_collection& collection ) {}
    };
} // namespace rnjin::ecs