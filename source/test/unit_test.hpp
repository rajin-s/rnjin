/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

// For unit test hpp files

#pragma once

#include <iostream>


#define SEPARATOR '/'
#define __FILENAME__ ( strrchr( __FILE__, SEPARATOR ) ? strrchr( __FILE__, SEPARATOR ) + 1 : __FILE__ )

class _unit_test
{
    public:
    virtual void run() = 0;
};

#define body \
    public:  \
    void run()


#define note( message ) std::cout << "    (" << __FILENAME__ << ": " << __LINE__ << ") " << message << std::endl
#define succeed( message ) std::cout << "< > (" << __FILENAME__ << ": " << __LINE__ << ") " << message << std::endl
#define fail( message ) std::cout << "<!> (" << __FILENAME__ << ": " << __LINE__ << ") " << message << std::endl

#define record( expression ) expression; note(#expression);
#define assert_equal( A, B ) if ( (A) == (B) ) { succeed( #A " = " #B ); } else { fail( #A " = " << (A) << " (expected " << (B) << ")" ); }

// For tests.generated.hpp
#define GET_TEST( name ) new unit_test__##name()

// For *.test.hpp
#define unit_test( name ) class unit_test__##name : public _unit_test
#define end_test