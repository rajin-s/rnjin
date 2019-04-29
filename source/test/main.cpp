#include <iostream>
#include "tests.hpp"

int main( int argc, char* argv[] )
{
    void ( *functions[] )( void ) = {
        TESTS
    };

    for (int i = 0; i < sizeof(functions) / sizeof(void*); i++)
    {
        functions[i]();
    }
    return 0;
}