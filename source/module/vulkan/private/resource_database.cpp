/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "resource_database.hpp"

namespace rnjin::graphics::vulkan
{
    resource_database::resource_database( renderer_internal& parent ) : child_class( parent ) {}
    resource_database::~resource_database() {}

    const resource_database::meshes::entry& resource_database::meshes::get( const resource::id resource_id ) const
    {
        let static invalid_entry = resource_database::meshes::entry( vk::Buffer(), vk::DeviceMemory() );

        check_error_condition( return invalid_entry, vulkan_log_errors, entries.count( resource_id ) == 0, "Failed to find Vulkan mesh resource with id=\1", resource_id );
        return entries.at( resource_id );
    }

    // const resource_database::meshes::entry resource_database::meshes::create( const mesh& mesh_resource )
    // {
        
    // }

} // namespace rnjin::graphics::vulkan