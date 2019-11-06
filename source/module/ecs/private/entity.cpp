/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "entity.hpp"

#include "component.hpp"

namespace rnjin::ecs
{
    log::source& get_ecs_log()
    {
        static log::source ecs_log(
            "rnjin.ecs", log::output_mode::immediately, log::output_mode::immediately,
            {
                { "verbose", (uint) log_flag::verbose, false },    //
                { "errors", (uint) log_flag::errors, true },       //
                { "entity", (uint) log_flag::entity, true },       //
                { "component", (uint) log_flag::component, true }, //
                { "system", (uint) log_flag::system, true },       //
            }                                                      //
        );
        return ecs_log;
    }

    log::source::masked ecs_log_verbose = get_ecs_log().mask( log_flag::verbose );
    log::source::masked ecs_log_errors  = get_ecs_log().mask( log_flag::errors );

    entity::entity()                //
      : destroying( false ) //
    {}
    entity::~entity()
    {
        destroying = true;
        
        ecs_log_verbose.print( "Destroy entity (\1), (\2 owned component types)", get_id(), owned_component_types.size() );
        foreach ( component_type_handle_pointer : owned_component_types )
        {
            component_type_handle_pointer->on_entity_destroyed( *this );
        }
    }

    void entity::add_component_type_handle( const component_type_handle_base* type_handle_pointer )
    {
        owned_component_types.insert( type_handle_pointer );
    }
    void entity::remove_component_type_handle( const component_type_handle_base* type_handle_pointer )
    {
        owned_component_types.erase( type_handle_pointer );
    }

    // define_static_group( entity::events );
} // namespace rnjin::ecs