/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>
#include <core.hpp>
#include <test.hpp>

using namespace rnjin;

int main( int argc, char* argv[] )
{
    try
    {
        for ( uint i : range( argc ) )
        {
            // Skip the first argument (executable name)
            if ( i == 0 ) continue;

            string test_name = string( argv[i] );

            if (test_name == "all")
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
    catch (std::exception e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}