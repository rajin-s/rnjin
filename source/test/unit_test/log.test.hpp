/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "unit_test.hpp"
#include <log.hpp>

using namespace rnjin;
using namespace rnjin::core;

unit_test( log )
{
    body
    {
        // if ( 1 + 1 == 2 ) pass("cool"); else fail("not cool");
        
        // test( 3 + 3 == 7, "bad", "good" );
        
        // test( 1 + 1 == 2 )
        //     pass("1 + 1 == 2")
        //     else fail("1 + 1 =/= 2")
        
        // test( 2 + 3 == 4 )
        //     pass( "2 + 3 == 4" )
        //     else fail( "2 + 3 =/= 4" )

        // list<string> printf_args = { "world" };
        // log::source output( "test.immediate", log::write_to_file::immediately );
        
        // output.printf( "Hello \1", printf_args );
        // output.printf( "Hello \1 \1 \1 \1", printf_args );
        
        // assert(1 == 1, "1 is 1 :)", "1 is not 1 :(");
    }
};