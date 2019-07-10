/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

// Common macros (these aren't that great...)
#define s( value ) std::to_string( value )
#define text( name, value ) const string message__##name = value
#define get_text( name ) message__##name

// C++ language extensions
#define foreach( iteration ) for ( const auto& iteration )
#define let const auto
#define group struct
#define is_abstract = 0
#define pass

/* clang-format off */
#define and &&
#define or ||
#define xor ^
#define not !
/* clang-format on */

// #define get_( func_name, var_name )    \
//     const auto get_##func_name() const \
//     {                                  \
//         return var_name;               \
//     }

// #define getref_( func_name, var_name )  \
//     const auto& get_##func_name() const \
//     {                                   \
//         return var_name;                \
//     }

// #define __lparen (
// #define __rparen )

// #define through ,
// #define __make_getter(var, name) const auto name() const { return var; }
// #define __make_ref_getter(var, name) const auto& name() const { return var; }
// #define make_value_accessible(var_as_name) __make_getter __lparen var_as_name __rparen
// #define make_reference_accessible(var_as_name) __make_ref_getter __lparen var_as_name __rparen

// #define returns ,
// #define __make_accessor(name, var) const auto name() const { return var; }
// #define __make_ref_accessor(name, var) const auto& name() const { return var; }
// #define accessor(name_returns_var) __make_accessor __lparen name_returns_var __rparen
// #define ref_accessor(name_returns_var) __make_ref_accessor __lparen name_returns_var __rparen

// #define accessor(method_name) const auto method_name() const
// #define ref_accessor(method_name) const auto& method_name() const

// #define get_(name) const auto get_##name() const
// #define returns(return_value) { return return_value; }

// #define accessor const auto
// #define returns(value) () const { return value; }

// #define access(value) () const { return value; }

// #define wrap_value(value) () const { return value; }

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

// #define __file_path_separator '\\'

// #define __raw_call_string __raw_file_string " @ <" __raw_function_string ":" __raw_line_string ">"
// #define _get_call_string() ( strrchr( __raw_call_string, __file_path_separator ) ? strrchr( __raw_call_string, __file_path_separator ) + 1 : __raw_call_string )