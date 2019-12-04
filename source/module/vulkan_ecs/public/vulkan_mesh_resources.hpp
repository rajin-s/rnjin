/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include third_party_library( "vulkan/vulkan.hpp" )

#include "vulkan_memory.hpp"

#include rnjin_module( core )
#include rnjin_module( reflection )
#include rnjin_module_subset( graphics, ecs )

namespace rnjin::graphics::vulkan
{
    /* -------------------------------------------------------------------------- */
    /*                                 Components                                 */
    /* -------------------------------------------------------------------------- */

    component_class( mesh_resources )
    {
        public: // methods
        mesh_resources();
        ~mesh_resources();

        public: // accessors
        let& get_vertices get_value( vertex_buffer_allocation );
        let& get_indices get_value( index_buffer_allocation );
        let get_index_count get_value( index_buffer_allocation.get_size() / sizeof( mesh::index ) );

        private: // members
        version_id current_vertices_version;
        version_id current_indices_version;

        buffer_allocation vertex_buffer_allocation;
        buffer_allocation index_buffer_allocation;

        friend class mesh_collector;
    };

    /* -------------------------------------------------------------------------- */
    /*                                   Systems                                  */
    /* -------------------------------------------------------------------------- */

    class mesh_collector                                                   //
      : public ecs::system<read_from<ecs_mesh>, write_to<mesh_resources>>, //
        public event_receiver                                              //
    {
        public: // methods
        mesh_collector( resource_database& resources );
        ~mesh_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_mesh_created( ecs_mesh& new_mesh, entity& owner );
        void on_mesh_destroyed( const ecs_mesh& old_mesh, entity& owner );

        private: // members
        resource_database& resources;
    };
    
    class mesh_reference_collector                                                               //
      : public ecs::system<read_from<ecs_mesh::reference>, write_to<mesh_resources::reference>>, //
        public event_receiver                                                                    //
    {
        public: // methods
        mesh_reference_collector();
        ~mesh_reference_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_mesh_reference_created( ecs_mesh::reference& new_mesh_reference, entity& owner );
        void on_mesh_reference_destroyed( const ecs_mesh::reference& old_mesh_reference, entity& owner );
    };
} // namespace rnjin::graphics::vulkan

/* -------------------------------------------------------------------------- */
/*                               Reflection Info                              */
/* -------------------------------------------------------------------------- */

namespace reflection
{
    auto_reflect_component( rnjin::graphics, vulkan::mesh_resources );
    auto_reflect_type( rnjin::graphics, vulkan::mesh_collector );
    auto_reflect_type( rnjin::graphics, vulkan::mesh_reference_collector );
} // namespace reflection