/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <rnjin.hpp>

#include rnjin_module( test )
#include rnjin_module( math )

using namespace rnjin;
using namespace rnjin::core;

test( vector_types )
{
    int2 int_vec2;
    record( int_vec2 = int2( 1, 3 ) );
    assert_equal( int_vec2.x, 1 );
    assert_equal( int_vec2.y, 3 );

    int3 int_vec3;
    record( int_vec3 = int3( 2, 4, 8 ) );
    assert_equal( int_vec3.x, 2 );
    assert_equal( int_vec3.y, 4 );
    assert_equal( int_vec3.z, 8 );

    int4 int_vec4;
    record( int_vec4 = int4( 255, 255, 255, 0 ) );
    assert_equal( int_vec4.x, 0xFF );
    assert_equal( int_vec4.y, 0xFF );
    assert_equal( int_vec4.z, 0xFF );
    assert_equal( int_vec4.w, 0x00 );

    float3 float_vec3;
    record( float_vec3 = float3( 1.0f, 2.4f, 3.333f ) );
    assert_equal( float_vec3.x, 1.0f );
    assert_equal( float_vec3.y, 2.4f );
    assert_equal( float_vec3.z, 3.333f );
}

test( vector_comparison )
{
    int2 a, b;
    record( a = int2( 1, 3 ) );
    assert_equal( a, int2( 1, 3 ) );

    record( b = int2( 3, 1 ) );
    assert_equal( b, int2( 3, 1 ) );

    assert_equal( ( a == b ), false );

    assert_equal( float3( 1, 0, 0 ), float3( 1.0, 0, 0 ) );
    assert_equal( float3( 1.1, 0, 0 ), float3( 1.1f, 0, 0 ) );
}

test( vector_operators )
{
    note( "Addition" );
    assert_equal( float2( 1, 3 ) + float2( 3, 4 ), float2( 4, 7 ) );
    assert_equal( float2( 1.2, 3.4 ) + float2( 5.6, 7.8 ), float2( 1.2f + 5.6f, 3.4f + 7.8f ) );
    assert_equal( double4( 1.2, 3.4, 5.6, 7.8 ) - double4( 0.9, 8.7, 6.5, 4.3 ), double4( 1.2 - 0.9, 3.4 - 8.7, 5.6 - 6.5, 7.8 - 4.3 ) );

    note( "Subtraction" );
    assert_equal( int3( 100, 100, 100 ) - int3( 100, 100, 100 ), int3() );
    assert_equal( float3( 100, 100, 100 ) - float3( 100, 100, 100 ), float3() );

    note( "Scalar Multiplication" );
    assert_equal( int2( 1, 5 ) * 3, int2( 3, 15 ) );
    assert_equal( 3 * int2( 1, 5 ), int2( 3, 15 ) );
    assert_equal( 2 * int2( 0, -5 ) * 3, int2( 0, -30 ) );

    note( "Conversion" );
    assert_equal( (float3) float2( 3, 4 ), float3( 3, 4, 0 ) );
}