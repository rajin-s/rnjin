/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "ecs.hpp"
#include "test.hpp"

using namespace rnjin::ecs;

// A simple component type with a single int value
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

// A simple component type with a single float value and a method to increase it
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

// A system that reads from an int component and increases a float component's value by that amount
class test_system_1 : public rnjin::ecs::system<read_from<int_component>, write_to<float_component>>
{
    protected: // inherited
    void define() override {}
    void update( entity_components& components ) override
    {
        let& i         = components.readable<int_component>();
        let_mutable& f = components.writable<float_component>();

        std::cout << "                          ";
        std::cout << "test_system_1: Add " << i.get_int_value() << " to float component (" << f.get_float_value() << ")" << std::endl;

        f.add_to_float_value( i.get_int_value() );
    }
};

test( ecs_execution )
{
    // Create entities and a test system
    entity ent1, ent2, ent3, ent4;
    test_system_1 sys;

    // Add int components to entities with constructor arguments
    record( ent4.add<int_component>( 4 ) );
    record( ent2.add<int_component>( 2 ) );
    record( ent1.add<int_component>( 1 ) );
    record( ent3.add<int_component>( 3 ) );

    // The system should do nothing, since none of the entities have both int and float components
    record( sys.update_all() );

    // Add float components to entities
    record( ent3.add<float_component>( 100.0 ) );
    record( ent1.add<float_component>( 0.0 ) );

    // Make sure initial values are set properly
    assert_equal( ent1.get<float_component>()->get_float_value(), 0.0 );
    assert_equal( ent3.get<float_component>()->get_float_value(), 100.0 );

    // The system now operates on ent1 and ent3
    // note: order is always based on creation order of entities
    record( sys.update_all() );

    // Make sure values are updated correctly
    assert_equal( ent1.get<float_component>()->get_float_value(), 1.0 );
    assert_equal( ent3.get<float_component>()->get_float_value(), 103.0 );

    // Remove the int component from ent1, now the system will no longer operate on it
    record( ent1.remove<int_component>() );
    record( sys.update_all() );

    // Make sure values are updated correctly for ent3, and remain the same for ent1
    assert_equal( ent1.get<float_component>()->get_float_value(), 1.0 );
    assert_equal( ent3.get<float_component>()->get_float_value(), 106.0 );
}

// A simple component type with just a public int value
component_class( dependent_component )
{
    public:
    int value;
};

// A system that sets up event handlers such that whenever int_component is added to an entity, dependent_component will be added as well
class test_system_2 : public rnjin::ecs::system<read_from<int_component>, write_to<dependent_component>>, event_receiver
{
    public:
    void initialize()
    {
        // Set up event handlers for whenever an int_component is added or removed from an entity
        handle_event( int_component::events.added(), &test_system_2::on_int_component_added );
        handle_event( int_component::events.removed(), &test_system_2::on_int_component_removed );
    }

    protected: // inherited
    void define() override {}
    void update( entity_components& components ) override
    {
        let& i         = components.readable<int_component>();
        let_mutable& d = components.writable<dependent_component>();

        std::cout << "                                ";
        std::cout << "test_system_2: dependent_component (" << d.value << " -> " << d.value + i.get_int_value() << ")" << std::endl;
        d.value += i.get_int_value();
    }

    private: // methods
    void on_int_component_added( int_component& c, entity& e )
    {
        std::cout << "                                ";
        if ( e.get<dependent_component>() == nullptr )
        {
            std::cout << "test_system_2: require dependent_component on entity (" << e.get_id() << ")" << std::endl;
        }
        else
        {
            std::cout << "test_system_2: dependent_component already attached to entity (" << e.get_id() << ")" << std::endl;
        }

        // Add dependent_component to the entity that an int_component was just added to
        // note: only adds the component if one is not already attached
        e.require<dependent_component>();
    }
    void on_int_component_removed( const int_component& c, entity& e )
    {
        std::cout << "                                ";
        std::cout << "test_system_2: remove dependent_component from entity (" << e.get_id() << ")" << std::endl;

        // Remove dependent_component to the entity that an int_component was just removed from
        // note: this just removes the component completely, since there is no 'unrequire' method
        e.remove<dependent_component>();
    }
};

test( ecs_initialization )
{
    // Create entities and a test system
    entity ent1, ent2, ent3, ent4;
    test_system_2 sys;

    // Register system event handlers
    sys.initialize();

    // Adding an int component will trigger the system's event handlers and add dependent components to ent2 and ent4
    record( ent4.add<int_component>( 4 ) );
    record( ent2.add<int_component>( 2 ) );

    // The system operates on ent2 and ent4 since dependent component was required by the system
    record( sys.update_all() );
    assert_equal( ent2.get<dependent_component>()->value, 2 );
    assert_equal( ent4.get<dependent_component>()->value, 4 );

    // dependent component is added first, so it isn't added again in the call to entity.require()
    record( ent1.add<dependent_component>() );
    record( ent1.add<int_component>( 1 ) );

    // The system now operates on ent1, ent2, and ent4
    record( sys.update_all() );
    assert_equal( ent1.get<dependent_component>()->value, 1 );
    assert_equal( ent2.get<dependent_component>()->value, 4 );
    assert_equal( ent4.get<dependent_component>()->value, 8 );

    // Add int component (and thus require dependent component) on ent4
    record( ent3.add<int_component>( 3 ) );

    // Removing int component from ent2 will trigger the system to also remove dependent component
    record( ent2.remove<int_component>() );

    // The system now operates on ent1, ent3, and ent4
    record( sys.update_all() );
    assert_equal( ent1.get<dependent_component>()->value, 2 );
    assert_equal( ent3.get<dependent_component>()->value, 3 );
    assert_equal( ent4.get<dependent_component>()->value, 12 );
}