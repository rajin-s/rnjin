/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <iostream>
#include "tests.generated.hpp"

int main( int argc, char* argv[] )
{
    _unit_test* tests[] = {
        TESTS
    };

    for (int i = 0; i < sizeof(tests) / sizeof(_unit_test*); i++)
    {
        tests[i]->run();
    }
    
    return 0;
}