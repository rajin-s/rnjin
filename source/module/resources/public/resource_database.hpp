/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "resource.hpp"

namespace rnjin::core
{
    // A resource database handles queries for loading a resource from file, and if it has already been loaded,
    // provides a reference to an existing shared resource object instead.
    // note: allocates new resources using 'new.' This could be simplified if resources were treated as components
    class resource_database : event_receiver
    {
        public: // methods
        resource_database();
        ~resource_database();

        template <typename T>
        static resource::reference<T> load( const string& file_path )
        {
            static resource_database db;

            let entry = db.entries.find( file_path );

            if ( entry == db.entries.end() )
            {
                // No resource has been previously loaded at this path

                // allocate a new resource
                T* new_resource = new T;

                // load data in from the given path
                new_resource->set_path( file_path );
                new_resource->force_reload();

                db.entries.emplace( file_path, new_resource );
                return resource::reference<T>( *new_resource );
            }
            else
            {
                // Some resource has already been loaded at this path
                return resource::reference<T>( *(T*) entry->second );
            }
        }

        private: // methods
        void on_resource_no_longer_referenced( const resource& old_resource );

        private: // members
        dictionary<string, resource*> entries;
    };
} // namespace rnjin::core