/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>

#include "core.hpp"
#include "log.hpp"
#include "script.hpp"

template <int V>
void print_num()
{
    log::main << "Instruction " << s( V ) << log::line();
}

template <typename T>
void print_bits( const void* value )
{
    size_t size = sizeof( T ) * 8;
    T as_t      = *( (T*)value );
    for ( int i = size - 1; i >= 0; i-- )
    {
        log::main << s( ( as_t >> i ) & 0x01 );
    }
    log::main << log::line();
}

void foo( char a, char b )
{
    log::main << "foo: " << s( a ) << ", " << s( b ) << log::line();
}

void bar( char a )
{
    log::main << "bar: " << s( a ) << log::line();
}

void foobar( void )
{
    log::main << "!";
}

void main( int argc, char* argv[] )
{
    char data[]    = { 0x04, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02 };
    auto foo_call  = script::function_call<void, char, char>( foo );
    auto foo_call2 = script::function_call( foo );

    foo_call.invoke( data );
    foo_call2.invoke( data );

    auto bar_call = script::function_call( bar );
    bar_call.invoke( data );

    auto foobar_call = script::function_call( foobar );
    foobar_call.invoke( data );
}