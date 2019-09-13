/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "containers.hpp"

// Common macros (these aren't that great...)
// #define s( value ) std::to_string( value )
// #define text( name, value ) const string message__##name = value
// #define get_text( name ) message__##name

// More descriptive forms of common patterns
#define group struct
#define subregion for ( bool __ = true; __; __ = false )
#define forward_declare_class( _namespace, _class ) \
    namespace _namespace                            \
    {                                               \
        class _class;                               \
    }

#define with( object )    \
    bool __with__ = true; \
    for ( object; __with__; __with__ = false )

#define goto_target( name ) \
    name:

// C++ language extensions
#define foreach( iteration ) for ( const auto& iteration )
#define let const auto
#define is_abstract = 0
#define pass
#define nonconst

/* clang-format off */
#define and &&
#define or ||
#define xor ^
#define not !
/* clang-format on */

// simple accessor macro to use with let
// example: let get_name get_value( name );
//          let& get_name get_value( name );
#define get_value( value ) \
    () const               \
    {                      \
        return value;      \
    }

// Introspection utilities
#define __STR2( x ) #x
#define __STR( x ) __STR2( x )

#define __raw_function_string __FUNCTION__
#define __raw_file_string __FILE__
#define __raw_line_string __STR( __LINE__ )