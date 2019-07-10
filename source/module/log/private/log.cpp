/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "log.hpp"

#include <iostream>
#include <sstream>

namespace rnjin
{
    namespace log
    {
        // Log icons for different print_* calls
        namespace icon
        {
            const string default = "    ";
            const string warning = "(?) ";
            const string error   = "<!> ";
            const uint size      = 4;
        } // namespace icon

        const string log_directory = "./logs/";
        const string log_extension = ".log";

        source main( "rnjin.main", output_mode::immediately, output_mode::immediately );

        source::source( const string& log_name, const output_mode console_output_mode, const output_mode file_output_mode )
        {
            name = log_name;

            // Save a string the same length as name but all blank for printing message additions
            name_blank.resize( name.size(), ' ' );

            // If a file is created by the log, this will be the name
            default_file_name = log_directory + log_name + log_extension;

            if ( console_output_mode != output_mode::never )
            {
                add_output( std::cout, console_output_mode );
            }

            if ( file_output_mode != output_mode::never )
            {
                default_file_output_stream.open( default_file_name, std::ios::out );
                if ( not default_file_output_stream.good() or not default_file_output_stream.is_open() )
                {
                    std::cerr << "Failed to open log file " << default_file_name << "\n";
                }

                add_output( default_file_output_stream, file_output_mode );
            }

            print( "Log Started" );
        }
        source::~source()
        {
            print( "Log Ended" );
            if ( default_file_output_stream.is_open() )
            {
                default_file_output_stream.flush();
                default_file_output_stream.close();
            }
        }

        void source::add_output( std::ostream& stream, const output_mode mode )
        {
            outputs.push_back( { stream, mode } );
        }

        void source::write_range( const char** begin, uint* count, const char* reset )
        {
            if ( begin == nullptr or count == 0 )
            {
                return;
            }

            foreach ( output : outputs )
            {
                output.stream.write( *begin, *count );
            }

            if ( reset != nullptr )
            {
                *begin = reset + sizeof( const char );
                *count = 0;
            }
        }

        void source::print_prefix( const string& icon, const bool show_name )
        {
            // Don't add a newline for the first print
            // note: this assumes nothing is printed *before* the first print_prefix call, which is probably fine?
            if ( first_output )
            {
                first_output = false;
            }
            else
            {
                write( "\n" );
            }

            write( icon );
            if ( show_name )
            {
                write( name );
                write( ": " );
            }
            else
            {
                write( name_blank );
                write( "  " );
            }
        }

        // Static log management
        dictionary<string, source> sources;
    } // namespace log
} // namespace rnjin