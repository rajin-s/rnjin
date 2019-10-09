/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "object.hpp"
#include "test.hpp"

using namespace rnjin;
using namespace rnjin::core;

class test_receiver : public object
{
    public:
    test_receiver(const string& name) : object(), name(name) {}
    ~test_receiver() {}

    void on_object_destroyed()
    {
        using namespace std;
        cout << name << ": Target object destroyed" << endl;
    }

    private:
    string name;
};

test( object_events )
{
    subregion
    {
        test_receiver obj("obj");

        subregion
        {
            object sender;
            obj.handle_event( sender.lifetime.on_destroyed(), &test_receiver::on_object_destroyed );
            //                ^-- event to handle           ^-- instance method to be called
        }

        subregion
        {
            test_receiver receiver("receiver");
            receiver.handle_event( obj.lifetime.on_destroyed(), &test_receiver::on_object_destroyed );
            //                     ^-- event to handle        ^-- instance method to be called

            obj.handle_event( receiver.lifetime.on_destroyed(), &test_receiver::on_object_destroyed );
            //                ^-- event to handle             ^-- instance method to be called
        }
    }
}