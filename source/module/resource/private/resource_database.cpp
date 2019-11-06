/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "resource_database.hpp"

namespace rnjin::core
{
    void free_resource( const resource* resource_pointer )
    {
        delete resource_pointer;
    }

    resource_database::resource_database()
    {
        handle_event( resource::events.resource_no_longer_referenced(), &resource_database::on_resource_no_longer_referenced );
    }
    resource_database::~resource_database()
    {
        for ( auto pair : entries )
        {
            let* resource_pointer = pair.second;
            if ( resource_pointer->has_references() )
            {
                // TODO: EMIT WARNING
            }

            free_resource( resource_pointer );
        }
    }

    void resource_database::on_resource_no_longer_referenced( const resource& old_resource )
    {
        let entry = entries.find( old_resource.get_path() );

        if ( entry != entries.end() )
        {
            // The resource is contained in this resource_database
            let* resource_pointer = entry->second;
            entries.erase( entry );

            free_resource( resource_pointer );
        }
    }
} // namespace rnjin::core