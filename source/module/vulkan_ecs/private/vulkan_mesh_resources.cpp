/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_mesh_resources.hpp"

namespace rnjin::graphics::vulkan
{
    /* -------------------------------------------------------------------------- */
    /*                               Mesh Resources                               */
    /* -------------------------------------------------------------------------- */

    mesh_resources::mesh_resources()                       //
      : current_vertices_version( version_id::invalid() ), //
        current_indices_version( version_id::invalid() )   //
    {}
    mesh_resources::~mesh_resources() {}

    /* -------------------------------------------------------------------------- */
    /*                               Mesh Collector                               */
    /* -------------------------------------------------------------------------- */

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

    /* -------------------------------------------------------------------------- */
    /*                          Mesh Reference Collector                          */
    /* -------------------------------------------------------------------------- */

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
} // namespace rnjin::graphics::vulkan