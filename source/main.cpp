/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>

#include "core.hpp"
#include "log.hpp"

void main( int argc, char* argv[] )
{
    log::source output( "engine.hello", log::write_to_file::immediately );
    output.printf( "Hello \1 and \2 and \3 and \4 and \1", { "World", "friends", "rajin", "world2" }, "default" );
    output.printf( "Hello \1", { "World!" }, "bad channel" );
    output.printf( "Hello \1", { "World!" }, -1 );

    output << log::start() << "This is a message. There are (" << s( argc ) << ") args." << log::line();
}