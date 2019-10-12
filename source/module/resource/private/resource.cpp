/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "resource.hpp"


namespace rnjin
{
    namespace core
    {
        resource::resource() {}
        resource::~resource() {}

        // Called from derived class write_data
        void resource::save_to( io::file& file )
        {
            let is_external = has_file();

            if ( is_external )
            {
                io::file_log_verbose.print( "Saving external resource '\1'", file_path );

                // Store an external resource flag and the file path in the parent file
                file.write_var( subresource_type::external );
                file.write_string( file_path );

                // Don't save external subresources, they need to be saved manually

                // Open the subresource file and store actual data
                // io::file resource_file( file_path, io::file::mode::write );
                // check_error_condition( return, io::file_log, not file.is_valid(), "Failed to open subresource file '\1' for saving", file_path );
                // write_data( resource_file );
            }
            else
            {
                io::file_log_verbose.print( "Saving internal resource" );

                // Store an internal flag and write data directly to the parent file
                file.write_var( subresource_type::internal );
                write_data( file );
            }
        }

        // Called from derived class read_data
        void resource::load_from( io::file& file )
        {
            // Read an internal/external flag from the parent file
            let resource_location = file.read_var<subresource_type>();
            let is_external       = resource_location == subresource_type::external;

            if ( is_external )
            {
                // Get the file path
                file_path = file.read_string();

                // Open the subresource file and read actual data
                io::file resource_file( file_path, io::file::mode::read );
                check_error_condition( return, io::file_log_errors, not file.is_valid(), "Failed to open subresource file '\1' for loading", file_path );
                read_data( resource_file );
            }
            else
            {
                // Read data directly from the parent file
                read_data( file );
            }
        }

        // Save a resource that has an associated file path
        void resource::save()
        {
            // let task = io::file_log_verbose.track_scope( "Saving resource file" );

            check_error_condition( return, io::file_log_errors, not has_file(), "Can't save resource with no file path" );

            io::file file( file_path, io::file::mode::write );
            check_error_condition( return, io::file_log_errors, not file.is_valid(), "Failed to open resource file '\1' for saving", file_path );

            // Write data directly to the file (ignoring internal/external for this, child resources might still be external)
            write_data( file );
        }

        // Load a resource that has an associated file path
        void resource::force_reload()
        {
            // let task = io::file_log_verbose.track_scope( "Loading resource file" );

            check_error_condition( return, io::file_log_errors, not has_file(), "Can't load resource with no file path" );

            io::file file( file_path, io::file::mode::read );
            check_error_condition( return, io::file_log_errors, not file.is_valid(), "Failed to open resource file '\1' for loading", file_path );

            // Read data directly from the file (ignoring internal/external for this, child resources might still be external)
            read_data( file );
        }

        // Set the resource file path
        // note: other stuff could probably be done here (delete existing files, reload, etc.)
        void resource::set_path( const string& new_path )
        {
            file_path = new_path;
        }

        // Virtual methods that do nothing for a base resource type
        void resource::write_data( io::file& file )
        {
            pass;
        }
        void resource::read_data( io::file& file )
        {
            pass;
        }

    } // namespace core
} // namespace rnjin