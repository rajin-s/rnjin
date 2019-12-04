/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include rnjin_module( ecs )
#include rnjin_module_subset( graphics, resources )
#include rnjin_module_public( resources, resource_database )

using namespace rnjin::ecs;

namespace rnjin::graphics
{
    component_class( model )
    {
        public: // methods
        model( const string& mesh_path, const string& material_path )
          : mesh_resource( resource_database::load<mesh>( mesh_path ).get_mutable() ),            // load model resources from file paths
            material_resource( resource_database::load<material>( material_path ).get_mutable() ) // load model resources from file paths
        {}

        public: // accessors
        let& get_mesh get_value( mesh_resource );
        let& get_material get_value( material_resource );

        let_mutable& get_mesh_mutable get_mutable_value( mesh_resource );
        let_mutable& get_material_mutable get_mutable_value( material_resource );

        private: // members
        mesh mesh_resource;
        material material_resource;

        group
        {
            float3 x, y, z;
        }
        basis;
    };
} // namespace rnjin::graphics

// reflection_info_for( rnjin::graphics, model )
// {
//     reflect_type_name( "model" );
// };