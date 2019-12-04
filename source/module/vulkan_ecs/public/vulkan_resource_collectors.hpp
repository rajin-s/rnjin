/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include third_party_library( "vulkan/vulkan.hpp" )

#include "vulkan_resources.hpp"

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

    component_class( material_resources )
    {
        public: // methods
        material_resources();
        ~material_resources();

        public: // accessors
        let& get_pipeline get_value( pipeline );

        private: // members
        version_id current_material_version;
        version_id current_uniforms_version;

        render_pipeline pipeline;
        buffer_allocation uniform_buffer_allocation;

        friend class material_collector;
    };

    component_class( model_resources )
    {
        public: // methods
        model_resources();
        ~model_resources();

        private: // members
        friend class model_collector;
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

    class material_collector                                                       //
      : public ecs::system<read_from<ecs_material>, write_to<material_resources>>, //
        public event_receiver                                                      //
    {
        public: // methods
        material_collector( resource_database& resources );
        ~material_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_material_created( ecs_material& new_material, entity& owner );
        void on_material_destroyed( const ecs_material& old_material, entity& owner );

        private: // members
        resource_database& resources;

        public: // TEMP
        vk::RenderPass temp_render_pass;
    };
    class material_reference_collector                                                                   //
      : public ecs::system<read_from<ecs_material::reference>, write_to<material_resources::reference>>, //
        public event_receiver                                                                            //
    {
        public: // methods
        material_reference_collector();
        ~material_reference_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_material_reference_created( ecs_material::reference& new_material_reference, entity& owner );
        void on_material_reference_destroyed( const ecs_material::reference& old_material_reference, entity& owner );
    };

    class model_collector //
      : public ecs::system<read_from<ecs_model>, write_to<model_resources>>,
        public event_receiver //
    {
        public: // methods
        model_collector();
        ~model_collector();

        void initialize();

        protected: // inherited
        void define() override;
        void update( entity_components& components ) override;

        private: // methods
        void on_model_created( ecs_model& new_model, entity& owner );
        void on_model_destroyed( const ecs_model& old_model, entity& owner );
    };

} // namespace rnjin::graphics::vulkan

/* -------------------------------------------------------------------------- */
/*                               Reflection Info                              */
/* -------------------------------------------------------------------------- */

namespace reflection
{
    auto_reflect_component( rnjin::graphics, vulkan::mesh_resources );
    auto_reflect_component( rnjin::graphics, vulkan::material_resources );
    auto_reflect_component( rnjin::graphics, vulkan::model_resources );

    auto_reflect_type( rnjin::graphics, vulkan::mesh_collector );
    auto_reflect_type( rnjin::graphics, vulkan::material_collector );
    auto_reflect_type( rnjin::graphics, vulkan::model_collector );
} // namespace reflection