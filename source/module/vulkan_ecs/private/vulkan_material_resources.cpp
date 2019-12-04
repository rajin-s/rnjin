/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_material_resources.hpp"

namespace rnjin::graphics::vulkan
{
    /* -------------------------------------------------------------------------- */
    /*                             Material Resources                             */
    /* -------------------------------------------------------------------------- */

    material_resources::material_resources()               //
      : current_material_version( version_id::invalid() ), //
        current_uniforms_version( version_id::invalid() )  //
    {}
    material_resources::~material_resources() {}


    /* -------------------------------------------------------------------------- */
    /*                             Material Collector                             */
    /* -------------------------------------------------------------------------- */

    material_collector::material_collector( resource_database& resources ) //
      : pass_member( resources )                                           //
    {}
    material_collector::~material_collector() {}

    // Initialization
    void material_collector::define() {}
    void material_collector::initialize()
    {
        this->handle_event( ecs_material::events.added(), &material_collector::on_material_created );
        this->handle_event( ecs_material::events.removed(), &material_collector::on_material_destroyed );
    }

    // Event Handlers
    void material_collector::on_material_created( ecs_material& new_material, entity& owner )
    {
        owner.add<material_resources>();
    }
    void material_collector::on_material_destroyed( const ecs_material& old_material, entity& owner )
    {
        owner.remove<material_resources>();
    }

    // System Methods
    void material_collector::update( entity_components& components )
    {
        let& source              = components.readable<ecs_material>();
        let_mutable& destination = components.writable<material_resources>();

        // Re-create a pipeline if the source material has changed since the last update
        // note: will always be called for the first update, since the saved version starts invalid
        let original_version = destination.current_material_version;
        if ( destination.current_material_version.update_to( source.get_version() ) )
        {
            vulkan_log_verbose.print( "'\1': update pipelines (version \2 -> \3)", reflection::get_type_name<material_collector>(), original_version, source.get_version() );

            // Release an existing pipeline if it has been created
            if ( destination.pipeline.is_valid() )
            {
                resources.free_pipeline( destination.pipeline );
            }
            destination.pipeline = resources.create_pipeline( *source.get_vertex_shader(), *source.get_fragment_shader(), temp_render_pass );

            // Release an existing uniform buffer if it has been created
            if ( destination.uniform_buffer_allocation.is_valid() )
            {
                resources.free_uniform_buffer( destination.uniform_buffer_allocation );
            }
            destination.uniform_buffer_allocation = resources.create_uniform_buffer( sizeof( ecs_material::instance_data ), &source.get_instance_data() );
            destination.current_uniforms_version  = version_id::invalid();

            // Bind the new uniform buffer allocation to the pipeline's descriptor set
            resources.bind_uniform_buffer( destination.pipeline, destination.uniform_buffer_allocation );
        }

        // Transfer uniform data if it has changed since the last update
        // note: will always be called for the first update, as well as any update where
        //       the material itself changed, since the saved version is set to invalid
        let original_uniforms_version = destination.current_uniforms_version;
        if ( destination.current_uniforms_version.update_to( source.get_instance_data_version() ) )
        {
            vulkan_log_verbose.print( "'\1': update uniforms (version \2 -> \3)", reflection::get_type_name<material_collector>(), original_uniforms_version, source.get_instance_data_version() );

            resources.transfer_uniform_buffer( ecs_material::get_instance_data_size(), &source.get_instance_data(), destination.uniform_buffer_allocation );
        }
    }


    /* -------------------------------------------------------------------------- */
    /*                        Material Reference Collector                        */
    /* -------------------------------------------------------------------------- */

    material_reference_collector::material_reference_collector() {}
    material_reference_collector::~material_reference_collector() {}

    // Initialization
    void material_reference_collector::define() {}
    void material_reference_collector::initialize()
    {
        this->handle_event( ecs_material::reference::events.added(), &material_reference_collector::on_material_reference_created );
        this->handle_event( ecs_material::reference::events.removed(), &material_reference_collector::on_material_reference_destroyed );
    }

    // Event Handlers
    void material_reference_collector::on_material_reference_created( ecs_material::reference& new_material_reference, entity& owner )
    {
        // When a material reference component is created, also add a material_resources reference that points to
        // the material_resources on the same owner (which is known to be added by on_material_created)
        let* reference_owner_pointer = &new_material_reference.get_referenced_owner();
        owner.add<material_resources::reference>( reference_owner_pointer );
    }
    void material_reference_collector::on_material_reference_destroyed( const ecs_material::reference& old_material_reference, entity& owner )
    {
        owner.remove<material_resources::reference>();
    }

    // System Methods
    void material_reference_collector::update( entity_components& components )
    {
        let& source              = components.readable<ecs_material::reference>();
        let_mutable& destination = components.writable<material_resources::reference>();

        let& source_owner      = source.get_referenced_owner();
        let& destination_owner = destination.get_referenced_owner();

        // Make sure the material_resources reference points to the material_resources owned by the same entity as the original ecs_material
        if ( source_owner.get_id() != destination_owner.get_id() )
        {
            destination.set_target_from_owner( &source_owner );
        }
    }

} // namespace rnjin::graphics::vulkan
