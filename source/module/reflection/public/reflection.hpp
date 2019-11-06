/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "core/public/containers.hpp"
#include "core/public/macro.hpp"

/* -------------------------------------------------------------------------- */
/*                         Unspecified Reflection Info                        */
/* -------------------------------------------------------------------------- */

template <typename T>
struct reflect
{
    static const rnjin::string& get_type_name()
    {
        static const rnjin::string& type_name = "unknown_type";
        return type_name;
    }
};

/* -------------------------------------------------------------------------- */
/*                                Helper Macros                               */
/* -------------------------------------------------------------------------- */

#define reflection_info_for( type_namespace, type_name ) \
    template <>                                          \
    struct reflect<type_namespace::type_name>

#define reflect_type_name( name )                     \
    static const rnjin::string& get_type_name()       \
    {                                                 \
        static const rnjin::string& type_name = name; \
        return type_name;                             \
    }