/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "mesh.hpp"
#include "material.hpp"
#include "ecs.hpp"

using namespace rnjin::ecs;

namespace rnjin::graphics
{
    component_class( model )
    {
        public: // methods
        model( const string& mesh_path, const string& material_path )
          : mesh_resource( resource::load<mesh>( mesh_path ) ),            // load model resources from file paths
            material_resource( resource::load<material>( material_path ) ) // load model resources from file paths
        {}

        public: // accessors
        let& get_mesh get_value( mesh_resource );
        let& get_material get_value( material_resource );

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