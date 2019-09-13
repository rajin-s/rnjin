/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "file.hpp"

namespace rnjin
{
    namespace core
    {
        class resource
        {
            public: // static methods
            // Create a resource and immediately load it from a path
            template <typename T>
            static T load( const string& path )
            {
                T new_resource;
                new_resource.file_path = path;
                new_resource.force_reload();
                return new_resource;
            }

            public: // methods
            resource();
            ~resource();

            void save_to( io::file& file );   // Called from derived class write_data
            void load_from( io::file& file ); // Called from derived class read_data

            void save();         // Save a resource that has an associated file path
            void force_reload(); // Load a resource that has an associated file path

            void set_path( const string& new_path ); // Set the resource file path

            public: // structures
            using id = uint;

            public: // accessors
            inline let get_id get_value( unique_id );
            let has_file get_value( not file_path.empty() );

            protected:
            // Virtual methods do that nothing for a base resource type
            virtual void write_data( io::file& file );
            virtual void read_data( io::file& file );

            private: // enums
            enum class subresource_type
            {
                internal = 'i',
                external = 'e'
            };

            private: // members
            string file_path;
            id unique_id;
        };
    } // namespace core
} // namespace rnjin