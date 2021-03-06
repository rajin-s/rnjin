/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */
#include <rnjin.hpp>

#include <iostream>

#include "core/module.h"
#include "console/module.h"
#include "test/module.h"

using namespace rnjin;

int main( int argc, char* argv[] )
{
    console::parse_arguments( { "-af", "params/test" } );

    try
    {
        for ( uint i : range( argc ) )
        {
            // Skip the first argument (executable name)
            if ( i == 0 ) continue;

            string test_name = string( argv[i] );

            if ( test_name == "all" )
            {
                test::execute_all_test_actions();
                break;
            }
            else
            {
                test::execute_test_action( test_name );
            }
        }
        std::cout << std::endl << "Finished tests" << std::endl;
    }
    catch ( std::exception e )
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}