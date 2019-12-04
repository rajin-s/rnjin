/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <rnjin.hpp>

#include "test/module.h"
#include "event.hpp"

using namespace rnjin;
using namespace rnjin::core;

class test_class
{
    public:
    test_class( const string& name ) : name( name ) {}

    void handler( const string& message )
    {
        std::cout << "      " << name << ": " << message << std::endl;
    }

    const string name;
};

test( events )
{
    event<const string&> test_event( "Test Event" );
    test_class instance_a( "Instance A" );
    test_class instance_b( "Instance B" );

    note( "Expecting 'Instance B: Message 1'" );
    note( "Expecting 'Instance A: Message 1'" );
    note( "Expecting 'Instance A: Message 2'" );

    subregion
    {
        let handler_a = event_handler( test_event, &instance_a, &test_class::handler );
        {
            let handler_b = event_handler( test_event, &instance_b, &test_class::handler );
            test_event.send( "Message 1" );
        }
        test_event.send( "Message 2" );
    }
    test_event.send( "Message 3" );
}