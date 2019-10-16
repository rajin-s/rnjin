/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "ecs.hpp"
#include "test.hpp"

using namespace rnjin::ecs;


component_class( int_component )
{
    public:
    int_component( int value ) : value( value ) {}
    ~int_component() {}

    public: // accessors
    let get_int_value get_value( value );

    private:
    int value;
};

component_class( float_component )
{
    public:
    float_component( float value ) : value( value ) {}
    ~float_component() {}

    void add_to_float_value( float amount )
    {
        value += amount;
    }

    public: // accessors
    let get_float_value get_value( value );

    private:
    float value;
};

class test_system : public rnjin::ecs::system<read_from<int_component>, write_to<float_component>>
{
    protected: // inherited
    void define() override {}
    void initialize() override {}
    void update( entity_components& components ) override
    {
        let& i         = components.readable<int_component>();
        let_mutable& f = components.writable<float_component>();

        std::cout << "                          ";
        std::cout << "test_system: Add " << i.get_int_value() << " to float component (" << f.get_float_value() << ")" << std::endl;
        
        f.add_to_float_value( i.get_int_value() );
    }
};

test( ecs_execution )
{

    entity ent1, ent2, ent3, ent4;
    test_system sys;

    record( ent4.add<int_component>( 4 ) );
    record( ent2.add<int_component>( 2 ) );
    record( ent1.add<int_component>( 1 ) );
    record( ent3.add<int_component>( 3 ) );

    record( ent3.add<float_component>( 100.0 ) );
    record( ent1.add<float_component>( 0.0 ) );

    assert_equal( ent1.get<float_component>()->get_float_value(), 0.0 );
    assert_equal( ent3.get<float_component>()->get_float_value(), 100.0 );

    record( sys.update_all() );

    assert_equal( ent1.get<float_component>()->get_float_value(), 1.0 );
    assert_equal( ent3.get<float_component>()->get_float_value(), 103.0 );
}