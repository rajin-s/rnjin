/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <rnjin.hpp>

#include rnjin_module( test )
#include "file.hpp"

using namespace rnjin;
using namespace rnjin::io;

struct a_struct
{
    int a, b, c;
    float d, e;
    char f, g;
    bitmask h, i;

    a_struct( int a, int b, int c, float d, float e, char f, char g, bitmask h, bitmask i ) : a( a ), b( b ), c( c ), d( d ), e( e ), f( f ), g( g ), h( h ), i( i ) {}
    a_struct() {}
};

test( file_io )
{
    a_struct struct_value( 999, 256, -23, 19.98, -20.19, 'x', 'r', bitmask( bits( 1, 2, 3 ) ), bitmask( bits( 4, 5, 6 ) ) );

    list<int> some_ints       = { 8, 6, 7, 5, 3, 0, 9 };
    list<string> some_strings = { "Hello", "World", "?!" };

    subregion
    {
        file write_file( "test/write", file::mode::write );

        write_file.write_var( 1 );
        write_file.write_var( 2 );
        write_file.write_var( 3 );

        write_file.write_var( struct_value );
        write_file.write_string( "Hello World" );
        write_file.write_string( "rnjin rnjin rnjin" );

        write_file.write_buffer( some_ints );
        write_file.write_buffer( some_strings );
    }

    subregion
    {
        file read_file( "test/write", file::mode::read );

        assert_equal( read_file.read_var<int>(), 1 );
        assert_equal( read_file.read_var<int>(), 2 );
        assert_equal( read_file.read_var<int>(), 3 );

        let read_value = read_file.read_var<a_struct>();
        assert_equal( read_value.a, struct_value.a );
        assert_equal( read_value.b, struct_value.b );
        assert_equal( read_value.c, struct_value.c );
        assert_equal( read_value.d, struct_value.d );
        assert_equal( read_value.e, struct_value.e );
        assert_equal( read_value.f, struct_value.f );
        assert_equal( read_value.g, struct_value.g );
        assert_equal( read_value.h, struct_value.h );
        assert_equal( read_value.i, struct_value.i );

        assert_equal( read_file.read_var<string>(), "Hello World" );
        assert_equal( read_file.read_var<string>(), "rnjin rnjin rnjin" );

        list<int> read_ints = read_file.read_buffer<int>();
        assert_equal( read_ints.size(), some_ints.size() );

        for ( uint i : range( some_ints.size() ) )
        {
            assert_equal( some_ints[i], read_ints[i] );
        }

        list<string> read_strings = read_file.read_buffer<string>();
        assert_equal( read_strings.size(), some_strings.size() );

        for ( uint i : range( some_strings.size() ) )
        {
            assert_equal( some_strings[i], read_strings[i] );
        }
    }
}

test( endian )
{
    const uint number = 0x01020304;
    let first         = ( (byte*) &number )[0];

    if ( first == 0x01 )
    {
        note( "System is Big-Endian" );
        assert_equal( first, 0x01 );
    }
    else
    {
        note( "System is Little-Endian" );
        assert_equal( first, 0x04 );
    }

    subregion // writing
    {
        file write_file( "test/write", file::mode::write );
        write_file.write_var( number );
        note( "Wrote 0x010203004" );
    }
    subregion
    {
        file read_file( "test/write", file::mode::read );
        let read_number = read_file.read_var<uint>();
        note( "Read uint" );

        assert_equal( number, read_number );
    }
}