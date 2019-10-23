/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_resource_collector.hpp"

namespace rnjin::graphics::vulkan
{
    resource_collector::resource_collector( const renderer& target_renderer )
      : pass_member( target_renderer ),           //
        resources( target_renderer.get_device() ) //
    {}
    resource_collector::~resource_collector() {}

    void resource_collector::define() {}
    void resource_collector::initialize( usize vertex_buffer_space, usize index_buffer_space, usize staging_buffer_space )
    {
        resources.initialize(vertex_buffer_space, index_buffer_space, staging_buffer_space);

        handle_event( model::events.added(), &resource_collector::on_model_added );
        handle_event( model::events.removed(), &resource_collector::on_model_removed );
    }

    void resource_collector::update( entity_components& components )
    {
        let& source              = components.readable<model>();
        let_mutable& destination = components.writable<internal_resources>();

        // Update Vulkan resources associated with the model if needed
        // note: internally checks if the vertices/indices/material version has changed since the last update
        destination.update_mesh_data( source.get_mesh(), resources );
        destination.update_material_data( source.get_material(), resources, target_renderer.get_render_pass() );
    }

    void resource_collector::on_model_added( model& new_model, const entity& owner )
    {
        vulkan_log_verbose.print( "Add vulkan::internal_resources to entity with model (\1)", owner.get_id() );
        owner.require<internal_resources>();
    }
    void resource_collector::on_model_removed( const model& old_model, const entity& owner )
    {
        vulkan_log_verbose.print( "Remove vulkan::internal_resources from entity with model (\1)", owner.get_id() );
        if ( let_mutable* ptr = owner.get_mutable<internal_resources>() )
        {
            ptr->release( resources );
        }
        owner.remove<internal_resources>();
    }
} // namespace rnjin::graphics::vulkan