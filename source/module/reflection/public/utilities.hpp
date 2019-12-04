/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "core/public/macro.hpp"
#include "core/public/containers.hpp"

namespace reflection
{
    template <typename T>
    static const rnjin::string& get_type_name()
    {
        static const rnjin::string& name = "unknown type";
        return name;
    }
} // namespace reflection

#define auto_reflect_type( type_namespace, type_name )              \
    template <>                                                     \
    const rnjin::string& get_type_name<type_namespace::type_name>() \
    {                                                               \
        static const rnjin::string& name = #type_name;              \
        return name;                                                \
    }
#define auto_reflect_component( type_namespace, type_name )                    \
    template <>                                                                \
    const rnjin::string& get_type_name<type_namespace::type_name>()            \
    {                                                                          \
        static const rnjin::string& name = #type_name;                         \
        return name;                                                           \
    }                                                                          \
    template <>                                                                \
    const rnjin::string& get_type_name<type_namespace::type_name::reference>() \
    {                                                                          \
        static const rnjin::string& name = #type_name "::reference";           \
        return name;                                                           \
    }

// /* -------------------------------------------------------------------------- */
// /*                         Unspecified Reflection Info                        */
// /* -------------------------------------------------------------------------- */

// template <typename T = void>
// struct reflect
// {
//     static const rnjin::string& get_type_name()
//     {
//         static const rnjin::string& type_name = "unknown_type";
//         return type_name;
//     }
// };

// /* -------------------------------------------------------------------------- */
// /*                                Helper Macros                               */
// /* -------------------------------------------------------------------------- */

// #define reflection_info_for( type_namespace, type_name ) \
//     template <>                                          \
//     struct reflect<type_namespace::type_name>

// #define reflect_type_name( name )                     \
//     static const rnjin::string& get_type_name()       \
//     {                                                 \
//         static const rnjin::string& type_name = name; \
//         return type_name;                             \
//     }

// #define auto_reflect_type( type_namespace, type_name )              \
//     template <>                                                     \
//     const rnjin::string& get_type_name<type_namespace::type_name>() \
//     {                                                               \
//         static const rnjin::string& type_name = #type_name;         \
//         return type_name;                                           \
//     }