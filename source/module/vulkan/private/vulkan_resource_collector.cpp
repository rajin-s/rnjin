/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_resource_collector.hpp"

namespace rnjin::graphics::vulkan
{
    void resource_collector::define() {}
    void resource_collector::initialize()
    {
        handle_event( model::events.added(), &resource_collector::on_model_added );
        handle_event( model::events.removed(), &resource_collector::on_model_removed );
    }

    void resource_collector::update( entity_components& components )
    {
        let& source              = components.readable<model>();
        let_mutable& destination = components.writable<internal_resources>();

        destination.update( source );
    }

    void resource_collector::on_model_added( model& new_model, const entity& owner )
    {
        vulkan_log_verbose.print( "Add vulkan::internal_resources to entity with model (\1)", owner.get_id() );
        owner.require<internal_resources>();
    }
    void resource_collector::on_model_removed( const model& old_model, const entity& owner )
    {
        vulkan_log_verbose.print( "Remove vulkan::internal_resources from entity with model (\1)", owner.get_id() );
        owner.remove<internal_resources>();
    }
} // namespace rnjin::graphics::vulkan