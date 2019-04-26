#include "unit_test.hpp"
#include for_test("log.hpp")

unit_test( test_log )
{
    using namespace rnjin;
    //log::source output( "test.immediate", log::write_to_file::immediately );
    log::source output ( "hi", log::write_to_file::immediately );
    finish();
}

int test_log()
{
    using namespace rnjin;
    return 0;
}