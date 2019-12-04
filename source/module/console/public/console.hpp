/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include rnjin_module( core )

namespace rnjin::console
{
    namespace internal
    {
        using flag_action            = void ( * )( void );
        using flag_parameters_action = void ( * )( const list<string>& );

        void add_flag( const string& flag, const string& alt, const string& description, flag_action action );
        void add_flag_parameters( const string& flag, const string& alt, const string& description, flag_parameters_action action, const list<string>& parameter_names );
    } // namespace internal

    void parse_arguments( const list<string> args );
} // namespace rnjin::console

// Abuse static constructors being called before main to bind arguments before program execution
#define bind_console_flag( long_name, short_name, description, function_name )                                \
    struct _generated_bind_##function_name                                                                    \
    {                                                                                                         \
        _generated_bind_##function_name()                                                                     \
        {                                                                                                     \
            rnjin::console::internal::add_flag( "--" long_name, "-" short_name, description, function_name ); \
        }                                                                                                     \
    } _generated_bind_##function_name##_instance

#define bind_console_parameters( long_name, short_name, description, function_name, ... )                                                 \
    struct _generated_bind_##function_name                                                                                                \
    {                                                                                                                                     \
        _generated_bind_##function_name()                                                                                                 \
        {                                                                                                                                 \
            rnjin::console::internal::add_flag_parameters( "--" long_name, "-" short_name, description, function_name, { __VA_ARGS__ } ); \
        }                                                                                                                                 \
    } _generated_bind_##function_name##_instance