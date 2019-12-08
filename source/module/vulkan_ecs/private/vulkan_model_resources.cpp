/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_model_resources.hpp"

namespace rnjin::graphics::vulkan
{
    /* -------------------------------------------------------------------------- */
    /*                               Model Resources                              */
    /* -------------------------------------------------------------------------- */

    model_resources::model_resources() {}
    model_resources::~model_resources() {}


    /* -------------------------------------------------------------------------- */
    /*                               Model Collector                              */
    /* -------------------------------------------------------------------------- */

    model_collector::model_collector() {}
    model_collector::~model_collector() {}

    void model_collector::define() {}
    void model_collector::initialize()
    {
        this->handle_event( ecs_model::events.added(), &model_collector::on_model_created );
        this->handle_event( ecs_model::events.removed(), &model_collector::on_model_destroyed );
    }
    void model_collector::on_model_created( ecs_model& new_model, entity& owner )
    {
        owner.add<model_resources>();
    }
    void model_collector::on_model_destroyed( const ecs_model& old_model, entity& owner )
    {
        owner.remove<model_resources>();
    }

    void model_collector::update( entity_components& components )
    {
        // log::main.print( "\1::update", reflection::get_type_name<model_collector>() );
    }

} // namespace rnjin::graphics::vulkan