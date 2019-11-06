/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <iostream>
#include "core.hpp"

namespace rnjin::test
{
    void add_test_action( const string& name, action test_action );
    void execute_test_action( const string& name );
    void execute_all_test_actions();
} // namespace rnjin::test

#define note( message ) std::cout << std::endl << "      (" << current_test_name << ": " << __LINE__ << ") " << message;
#define _fail( message ) std::cout << std::endl << "  [!] (" << current_test_name << ": " << __LINE__ << ") " << message;
#define _pass( message ) std::cout << std::endl << "  [ ] (" << current_test_name << ": " << __LINE__ << ") " << message;
#define print_line( message ) std::cout << std::endl << "      (:" << __LINE__ << ") " << message;

#define record( expression ) \
    note( #expression );     \
    expression;

#define assert_equal( A, B )                                         \
    if ( ( A ) == ( B ) )                                            \
    {                                                                \
        _pass( #A " = " #B );                                        \
    }                                                                \
    else                                                             \
    {                                                                \
        _fail( #A " = " << ( A ) << " (expected " << ( B ) << ")" ); \
    }

#define test( name )                                                                         \
    namespace                                                                                \
    {                                                                                        \
        struct test_##name##_container                                                       \
        {                                                                                    \
            static constexpr char* current_test_name = "" #name;                             \
            static void run();                                                               \
            test_##name##_container()                                                        \
            {                                                                                \
                std::cout << "Registered test '" #name "' from '" __FILE__ "'" << std::endl; \
                rnjin::test::add_test_action( #name, run );                                  \
            }                                                                                \
        };                                                                                   \
        static test_##name##_container _test_##name;                                         \
    }                                                                                        \
    void ::test_##name##_container::run()