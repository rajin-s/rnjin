/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

/* -------------------------------------------------------------------------- */
/*                         Module / Include Utilities                         */
/* -------------------------------------------------------------------------- */

// clang-format off

#define __make_string( s ) #s

#define rnjin_module( module_name ) __make_string( module_name/module_name.hpp )
#define rnjin_module_subset( module_name, subset_name ) __make_string( module_name/subset_name.hpp )
#define rnjin_module_public( module_name, header_name ) __make_string( module_name/public/header_name.hpp )
#define internal_header( header_name ) __make_string( header_name.hpp )
#define standard_library( header_name ) <header_name>
#define third_party_library( header_name ) header_name

// clang-format on

/* -------------------------------------------------------------------------- */
/*                             Language Extensions                            */
/* -------------------------------------------------------------------------- */

// Descriptive forms of common patterns
// clang-format off
#define and &&
#define or ||
#define xor ^
#define not !
// clang-format on

#define group struct
#define define_static_group( name ) \
    decltype( name ) name {}

#define subregion for ( bool __ = true; __; __ = false )
#define forward_declare_class( _namespace, _class ) \
    namespace _namespace                            \
    {                                               \
        class _class;                               \
    }

#define foreach( iteration ) for ( const auto& iteration )
#define pass

#define nonconst
#define let const auto
#define let_mutable auto
#define cast( arg, type ) reinterpret_cast<type>( arg )

// Constructors and such

// clang-format off
#define pure_virtual =0
// clang-format on

#define pass_member( name ) name( name )
#define no_copy( class_name )                 \
    class_name( const class_name& ) = delete; \
    void operator=( const class_name& ) = delete;

// simple accessor macro to use with let
// example: let get_name get_value( name );
//          let& get_name get_value( name );
#define get_value( value ) \
    () const               \
    {                      \
        return value;      \
    }
#define get_mutable_value( value ) \
    ()                             \
    {                              \
        return value;              \
    }