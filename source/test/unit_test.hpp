/* *** ** *** ** *** ** *** *
* Part of rnjin            *
* (c) Rajin Shankar, 2019  *
*        rajinshankar.com  *
* *** ** *** ** *** ** *** */

// For unit test hpp files
#include <iostream>
using namespace std;

#define unit_test( name ) class unit_test_##name
#define body \
    public:  \
    static void run()
#define assert( condition, if_pass, if_fail ) \
    if ( condition )                          \
    {                                         \
        cout << if_pass << endl;              \
    }                                         \
    else                                      \
    {                                         \
        cout << if_fail << endl;              \
    }

// For tests.hpp
#define test( name ) unit_test_##name::run