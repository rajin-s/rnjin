/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "test.hpp"

namespace rnjin::test
{
    dictionary<string, action>& get_test_actions()
    {
        static dictionary<string, action> test_actions;
        return test_actions;
    }

    void add_test_action( const string& name, action test_action )
    {
        get_test_actions()[name] = test_action;
    }

    void execute_test_action( const string& name )
    {
        let actions = get_test_actions();
        if ( actions.count( name ) > 0 )
        {
            std::cout << std::endl << "Executing test '" << name << "'" << std::endl;
            actions.at( name )();
            std::cout << std::endl << "Finished test '" << name << "'" << std::endl;
        }
        else
        {
            std::cout << std::endl << "Failed to find test '" << name << "'" << std::endl;
        }
    }

    void execute_all_test_actions()
    {
        foreach ( pair : get_test_actions() )
        {
            pair.second();
        }
    }
} // namespace rnjin::test