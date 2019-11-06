/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "graphics.hpp"
#include "mesh.hpp"
#include "material.hpp"

namespace rnjin
{
    namespace graphics
    {
        // The intermediate data format between logical and rendering parts of the engine
        // A render_view should be translatable to rendering commands for any API
        class render_view
        {
            private: // structures
            struct item
            {
                item( const mesh& mesh_resource, const material& material_resource ) : pass_member( mesh_resource ), pass_member( material_resource ){};
                const mesh& mesh_resource;
                const material& material_resource;
            };

            public: // methods
            void add_item( const mesh& mesh_resource, const material& material_resource )
            {
                items.emplace_back( mesh_resource, material_resource );
            }

            public: // accessors
            let& get_items get_value( items );

            private: // members
            list<item> items;
        };
    } // namespace graphics
} // namespace rnjin
