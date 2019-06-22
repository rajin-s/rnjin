/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "unit_test.hpp"
#include <bitmask.hpp>

using namespace rnjin;
using namespace rnjin::core;

unit_test( bitmask )
{
    body
    {
        const int
            b0 = bit(0),
            b1 = bit(1),
            b2 = bit(2),
            b3 = bit(3),
            b4 = bit(4),
            b5 = bit(5),
            b6 = bit(6),
            b7 = bit(7);
        
        assert_equal( bitmask::none(), -0u );
        assert_equal( bitmask::all(), ~0u );
        assert_equal( bitmask::none(), ~bitmask::all() );

        assert_equal( bitmask::none() + 0, 0x01 );
        assert_equal( bitmask::none() + 1, 0x02 );
        assert_equal( bitmask::none() + 4, 0x10 );

        bitmask A;
        record( A = bitmask( bit(0) | bit(1) | bit(2) ) );

        assert_equal( A, 0x07 );
        assert_equal( A - 1, 0x05 );
        assert_equal( A -= 0, 0x06 );
        assert_equal( A += 0, 0x07 );

        record( A -= 1 );
        assert_equal( A, 0x05 );
        record( A += 4 );
        assert_equal( A, 0x15 );

        bitmask B;
        record( B = bitmask( b0 | b2 | b4 ) );

        assert_equal( A, B );
        assert_equal( A / B, bitmask::none() );
        record( A += 1 );
        assert_equal( A / B, bitmask( b1 ) );

        record( A = bitmask( b1 | b2 | b4 | b6 ) );
        record( B = bitmask( b0 | b1 | b2 | b7 ) );
        assert_equal( A & B, bitmask( b1 | b2 ) );
    }
};