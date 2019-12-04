/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_resource_collectors.hpp"

namespace rnjin::graphics::vulkan
{

/* -------------------------------------------------------------------------- */
/*                               Mesh Resources                               */
/* -------------------------------------------------------------------------- */
#pragma region mesh_resources

    mesh_resources::mesh_resources()                       //
      : current_vertices_version( version_id::invalid() ), //
        current_indices_version( version_id::invalid() )   //
    {}
    mesh_resources::~mesh_resources() {}

#pragma endregion mesh_resources

/* -------------------------------------------------------------------------- */
/*                               Mesh Collector                               */
/* -------------------------------------------------------------------------- */
#pragma region mesh_collector

    mesh_collector::mesh_collector( resource_database& resources ) //
      : pass_member( resources )                                   //
    {}
    mesh_collector::~mesh_collector() {}

    // Initialization
    void mesh_collector::define() {}
    void mesh_collector::initialize()
    {
        this->handle_event( ecs_mesh::events.added(), &mesh_collector::on_mesh_created );
        this->handle_event( ecs_mesh::events.removed(), &mesh_collector::on_mesh_destroyed );
    }

    // Event Handlers
    void mesh_collector::on_mesh_created( ecs_mesh& new_mesh, entity& owner )
    {
        owner.add<mesh_resources>();
    }
    void mesh_collector::on_mesh_destroyed( const ecs_mesh& old_mesh, entity& owner )
    {
        owner.remove<mesh_resources>();
    }

    // System Methods
    void mesh_collector::update( entity_components& components )
    {
        let& source              = components.readable<ecs_mesh>();
        let_mutable& destination = components.writable<mesh_resources>();

        // Re-allocate a vertex buffer if the source vertices have changed since the last update
        // note: will always be called for the first update, since the saved version starts invalid
        if ( destination.current_vertices_version.update_to( source.vertices.get_version() ) )
        {
            // Release an existing vertex buffer if it has been allocated
            // TODO: re-use the same buffer and copy new data if the number of elements stays the same
            if ( destination.vertex_buffer_allocation.is_valid() )
            {
                resources.free_vertex_buffer( destination.vertex_buffer_allocation );
            }

            destination.vertex_buffer_allocation = resources.create_vertex_buffer( source.vertices.get_data() );
        }

        // Re-allocate an index buffer if the source indices have changed since the last update
        // note: will always be called for the first update, since the saved version starts invalid
        if ( destination.current_indices_version.update_to( source.indices.get_version() ) )
        {
            // Release an existing index buffer if it has been allocated
            // TODO: re-use the same buffer and copy new data if the number of elements stays the same
            if ( destination.index_buffer_allocation.is_valid() )
            {
                resources.free_index_buffer( destination.index_buffer_allocation );
            }

            destination.index_buffer_allocation = resources.create_index_buffer( source.indices.get_data() );
        }
    }

#pragma endregion mesh_collector

/* -------------------------------------------------------------------------- */
/*                          Mesh Reference Collector                          */
/* -------------------------------------------------------------------------- */
#pragma region mesh_reference_collector

    mesh_reference_collector::mesh_reference_collector() {}
    mesh_reference_collector::~mesh_reference_collector() {}

    // Initialization
    void mesh_reference_collector::define() {}
    void mesh_reference_collector::initialize()
    {
        this->handle_event( ecs_mesh::reference::events.added(), &mesh_reference_collector::on_mesh_reference_created );
        this->handle_event( ecs_mesh::reference::events.removed(), &mesh_reference_collector::on_mesh_reference_destroyed );
    }

    // Event Handlers
    void mesh_reference_collector::on_mesh_reference_created( ecs_mesh::reference& new_mesh_reference, entity& owner )
    {
        // When a mesh reference component is created, also add a mesh_resources reference that points to
        // the mesh_resources on the same owner (which is known to be added by on_mesh_created)
        let* reference_owner_pointer = &new_mesh_reference.get_referenced_owner();
        owner.add<mesh_resources::reference>( reference_owner_pointer );
    }
    void mesh_reference_collector::on_mesh_reference_destroyed( const ecs_mesh::reference& old_mesh_reference, entity& owner )
    {
        owner.remove<mesh_resources::reference>();
    }

    // System Methods
    void mesh_reference_collector::update( entity_components& components )
    {
        let& source              = components.readable<ecs_mesh::reference>();
        let_mutable& destination = components.writable<mesh_resources::reference>();

        let& source_owner      = source.get_referenced_owner();
        let& destination_owner = destination.get_referenced_owner();

        // Make sure the mesh_resources reference points to the mesh_resources owned by the same entity as the original ecs_mesh
        if ( source_owner.get_id() != destination_owner.get_id() )
        {
            destination.set_target_from_owner( &source_owner );
        }
    }

#pragma endregion mesh_reference_collector


/* -------------------------------------------------------------------------- */
/*                             Material Resources                             */
/* -------------------------------------------------------------------------- */
#pragma region material_resources

    material_resources::material_resources()               //
      : current_material_version( version_id::invalid() ), //
        current_uniforms_version( version_id::invalid() )  //
    {}
    material_resources::~material_resources() {}

#pragma endregion material_resources

/* -------------------------------------------------------------------------- */
/*                             Material Collector                             */
/* -------------------------------------------------------------------------- */
#pragma region material_collector

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

#pragma endregion material_collector

/* -------------------------------------------------------------------------- */
/*                        Material Reference Collector                        */
/* -------------------------------------------------------------------------- */
#pragma region material_reference_collector

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

#pragma endregion material_reference_collector


/* -------------------------------------------------------------------------- */
/*                               Model Resources                              */
/* -------------------------------------------------------------------------- */
#pragma region model_resources

    model_resources::model_resources() {}
    model_resources::~model_resources() {}


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
        log::main.print( "\1::update", reflection::get_type_name<model_collector>() );
    }

#pragma endregion model_resources


} // namespace rnjin::graphics::vulkan