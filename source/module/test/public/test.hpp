/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <iostream>

#define note( message ) std::cout << "      (" << current_test_name << ": " << __LINE__ << ") " << message << std::endl
#define _fail( message ) std::cout << "  [!] (" << current_test_name << ": " << __LINE__ << ") " << message << std::endl
#define _pass( message ) std::cout << "  [ ] (" << current_test_name << ": " << __LINE__ << ") " << message << std::endl

#define record( expression ) \
    expression;              \
    note( #expression );

#define assert_equal( A, B )                                         \
    if ( ( A ) == ( B ) )                                            \
    {                                                                \
        _pass( #A " = " #B );                                        \
    }                                                                \
    else                                                             \
    {                                                                \
        _fail( #A " = " << ( A ) << " (expected " << ( B ) << ")" ); \
    }

#define test( name )                                                                \
    struct test_##name##_container                                                  \
    {                                                                               \
        static constexpr char* current_test_name = "" #name;                        \
        void run();                                                                 \
        test_##name##_container()                                                   \
        {                                                                           \
            std::cout << "Running test '" #name "' in '" __FILE__ "'" << std::endl; \
            run();                                                                  \
        }                                                                           \
    };                                                                              \
    static test_##name##_container _test_##name;                                    \
    void test_##name##_container::run()