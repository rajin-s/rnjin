/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "log.hpp"

#include <iostream>
#include <sstream>

#include "console/module.h"

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

        const string& get_log_directory()
        {
            static string value = "./logs/";
            return value;
        }
        const string& get_log_extension()
        {
            static string value = ".log";
            return value;
        }

        // Static log management
        static dictionary<string, source*>& get_sources()
        {
            static dictionary<string, source*> sources;
            return sources;
        }

        enum class log_flag
        {
            default = 0,
            verbose = 1,
            errors  = 2
        };

        source main(
            "rnjin.main",
            output_mode::immediately,
            output_mode::immediately,
            {
                { "default", (uint) log_flag::default, true },
                { "verbose", (uint) log_flag::verbose, false },
                { "errors", (uint) log_flag::errors, true },
            } // flags
        );

        source::masked main_verbose = main.mask( log_flag::verbose );
        source::masked main_errors  = main.mask( log_flag::errors );

        // Constructors
        source::source( const string& log_name, const output_mode console_output_mode, const output_mode file_output_mode )
        {
            name = log_name;

            // Save a string the same length as name but all blank for printing message additions
            name_blank.resize( name.size(), ' ' );

            // If a file is created by the log, this will be the name
            default_file_name = get_log_directory() + log_name + get_log_extension();

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

            // Output no flagged messages by default (unflagged will still go through)
            flag_output_mask = bitmask::none();

            // Track this in the global source list
            auto& sources = get_sources();
            check_error_condition( pass, log::main_errors, sources.count( log_name ) != 0, "A log source named '\1' already exists", log_name );
            sources[log_name] = this;

            // Record that the log has started
            print( "Log Started (write to '\1')", default_file_name );
        }
        source::source( const string& log_name, const output_mode console_output_mode, const output_mode file_output_mode, const list<flag_info> flags )
          : source( log_name, console_output_mode, file_output_mode )
        {
            foreach ( info : flags )
            {
                name_flag( info.number, info.name );
                if ( info.enabled )
                {
                    enable_flag( info.number );
                }
            }
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

        // Management
        void source::add_output( std::ostream& stream, const output_mode mode )
        {
            outputs.push_back( { stream, mode } );
        }

        void source::enable_flag( const uint number )
        {
            flag_output_mask += number;
        }
        void source::disable_flag( const uint number )
        {
            flag_output_mask -= number;
        }

        void source::name_flag( const uint number, const string& name )
        {
            named_flags[name] = number;
        }
        const uint source::get_flag_number( const string& flag_name ) const
        {
            check_error_condition( return invalid_flag, log::main_errors, named_flags.count( flag_name ) == 0, "Flag with name '\1' not found for log '\2'", flag_name, name );

            return named_flags.at( flag_name );
        }

        // General methods
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
            write( "\n" );
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

        // Console bindings
        void set_log_flag( const list<string>& args )
        {
            let& log_name  = args[0];
            let& action    = args[1];
            let& flag_name = args[2];

            let& sources = get_sources();
            check_error_condition( return, log::main_errors, sources.count( log_name ) == 0, "Invalid log name '\1'", log_name );

            source* target         = sources.at( log_name );
            let target_flag_number = target->get_flag_number( flag_name );
            if ( target_flag_number == source::invalid_flag )
            {
                return;
            }

            if ( action == "+" )
            {
                target->enable_flag( target_flag_number );
            }
            else if ( action == "-" )
            {
                target->disable_flag( target_flag_number );
            }
            else
            {
                const bool invalid_action = true;
                check_error_condition( return, log::main_errors, invalid_action == true, "Invalid action for log-flag '\1' (must be + or -)", action );
            }
        }

        bind_console_parameters( "log-flag", "lf", "Set or unset a log flag", set_log_flag, "log name", "+/-", "channel name" );
    } // namespace log
} // namespace rnjin

std::ostream& operator<<( std::ostream& stream, const rnjin::version_id& id )
{
    if ( id.is_invalid() )
    {
        stream << "invalid id";
    }
    else
    {
        stream << ( (rnjin::usize) id );
    }
    return stream;
}