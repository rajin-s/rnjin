/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>

#include "core.hpp"
#include "log.hpp"
#include "script.hpp"

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

void print_char( const char c )
{
    log::main.printf( "\1", { s( c ) } );
}

void main( int argc, char* argv[] )
{
    script::runtime rt = script::runtime();
    script::compiled_script test_script( "test.script" );
    script::execution_context exec( test_script, rt );

    exec.execute(); exec.execute();
    exec.execute(); exec.execute();
    exec.execute(); exec.execute();
    exec.execute(); exec.execute();
}