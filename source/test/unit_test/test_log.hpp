#include "unit_test.hpp"
#include <log.hpp>

unit_test( log )
{
    body
    {
        list<string> printf_args = { "world" };
        log::source output( "test.immediate", log::write_to_file::immediately );
        output.printf( "Hello \1", printf_args );
        output.printf( "Hello \1 \1 \1 \1", printf_args );
        assert(1 == 1, "1 is 1 :)", "1 is not 1 :(");
    }
};