/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "console.hpp"

#include <iostream>
#include "file/module.h"
#include "log/module.h"

namespace rnjin
{
    namespace console
    {
        // Console log flag constants
        enum class log_flag : uint
        {
            verbose = 2,
            errors  = 3
        };

        // Get a reference to the console log singleton (prefer masked sources so flags can be used)
        static log::source& get_console_log()
        {
            static log::source console_log(
                "rnjin.console", log::output_mode::immediately, log::output_mode::immediately,
                {
                    { "verbose", (uint) log_flag::verbose, false },
                    { "errors", (uint) log_flag::errors, true },
                } //
            );
            return console_log;
        }

        // Masked log sources
        log::source::masked console_log_verbose = get_console_log().mask( log_flag::verbose );
        log::source::masked console_log_errors  = get_console_log().mask( log_flag::errors );

        namespace internal
        {
            // Contains a function pointer and information for printing usage
            struct flag_binding
            {
                flag_action action;

                string alt;
                string description;

                bool is_alt;
            };

            // Contains a function pointer and information for printing usage
            struct flag_parameters_binding
            {
                flag_parameters_action action;
                uint parameter_count;

                string alt;

                list<string> parameter_names;
                string description;

                bool is_alt;
            };

            // Get the static flag binding dictionary
            // note: accessing through static methods ensures the dictionary is initialized before it first needs to be used
            //       (which might be during the initialization of other static data)
            static dictionary<string, flag_binding>& get_flag_bindings()
            {
                static dictionary<string, flag_binding> flag_bindings = {};
                return flag_bindings;
            }

            // Get the static flag w/ parameters binding dictionary
            // note: accessing through static methods ensures the dictionary is initialized before it first needs to be used
            //       (which might be during the initialization of other static data)
            static dictionary<string, flag_parameters_binding>& get_flag_parameters_bindings()
            {
                static dictionary<string, flag_parameters_binding> flag_parameters_bindings = {};
                return flag_parameters_bindings;
            }

            // Register a flag that takes no parameters with appropriate help info
            // note: should be called from bind_console_flag macro
            static const string empty_alt_flag = "-";
            void add_flag( const string& flag, const string& alt, const string& description, flag_action action )
            {
                flag_binding new_binding{ action, alt, description, false };
                get_flag_bindings().insert_or_assign( flag, new_binding );

                if ( alt != empty_alt_flag )
                {
                    flag_binding alt_binding{ action, "", "", true };
                    get_flag_bindings().insert_or_assign( alt, alt_binding );
                }
            }

            // Register a flag that takes parameters with appropriate help info
            // note: should be called from bind_console_parameters macro
            void add_flag_parameters( const string& flag, const string& alt, const string& description, flag_parameters_action action, const list<string>& parameter_names )
            {
                const uint parameter_count = parameter_names.size();
                flag_parameters_binding new_binding{ action, parameter_count, alt, parameter_names, description, false };
                get_flag_parameters_bindings().insert_or_assign( flag, new_binding );

                if ( alt != empty_alt_flag )
                {
                    flag_parameters_binding alt_binding{ action, parameter_count, "", parameter_names, "", true };
                    get_flag_parameters_bindings().insert_or_assign( alt, alt_binding );
                }
            }
        } // namespace internal

        // Parse a list of arguments, consuming them as appropriate
        // (bindings with n parameters will consume n+1 arg tokens)
        void parse_arguments( const list<string> args )
        {
            auto& flag_bindings            = internal::get_flag_bindings();
            auto& flag_parameters_bindings = internal::get_flag_parameters_bindings();

            console_log_verbose.print( "Parsing \1 arguments (\2 bindings)", args.size(), flag_bindings.size() + flag_parameters_bindings.size() );

            let arg_count = args.size();
            for ( uint i = 0; i < arg_count; i++ )
            {
                let& arg = args[i];

                let binding_count = flag_parameters_bindings.count( arg ) + flag_bindings.count( arg );
                check_error_condition( continue, console_log_errors, binding_count == 0, "No binding found for '\1'", arg );

                // Handle actions that consume the next argument(s)
                if ( flag_parameters_bindings.count( arg ) > 0 )
                {
                    let binding   = flag_parameters_bindings[arg];
                    let args_left = arg_count - 1 - i;

                    console_log_verbose.print( "Executing flag '\1' with \2 arguments (\3 left)", arg, binding.parameter_count, args_left );

                    // Check that enough arguments are left to satisfy the binding
                    if ( args_left >= binding.parameter_count )
                    {
                        list<string> parameters( &args[i + 1], &args[i + binding.parameter_count + 1] );
                        console_log_verbose.print_additional( "call with \1 params (", parameters.size() );

                        for ( uint i : range( parameters.size() ) )
                        {
                            console_log_verbose << parameters[i];
                            if ( i != parameters.size() - 1 )
                            {
                                console_log_verbose << ", ";
                            }
                        }
                        console_log_verbose << ")";

                        binding.action( parameters );
                        i += binding.parameter_count;
                    }
                    else
                    {
                        // Report that not enough arguments were left and display correct usage
                        console_log_verbose.print_warning( "Flag '\1' recieved \2 parameters, expected \3 [", arg, args_left, binding.parameter_count );
                        for ( uint i : range( binding.parameter_count ) )
                        {
                            console_log_verbose << binding.parameter_names[i];
                            if ( i != binding.parameter_count - 1 )
                            {
                                console_log_verbose << ", ";
                            }
                        }
                        console_log_verbose << "]";
                    }
                }

                // Handle parameterless flag actions
                if ( flag_bindings.count( arg ) > 0 )
                {
                    console_log_verbose.print( "Executing flag '\1'", arg );
                    flag_bindings[arg].action();
                }
            }
        }

        // Read a sequence of arguments from a file, separated by space or newlines
        void parse_arguments_from_file( const list<string>& args )
        {
            let& path = args[0];

            io::file file( path, io::file::mode::read );
            check_error_condition( return, console_log_errors, not file.is_valid(), "Failed to open arguments file '\1'", path );

            const string text = file.read_all_text();
            list<string> read_args;

            size_t position = 0;
            while ( position != string::npos and position < text.length() )
            {
                size_t skip = 1;

                size_t next;
                const size_t next_space       = text.find( ' ', position + 1 );
                const size_t next_line        = text.find( '\n', position + 1 );
                const size_t next_return_line = text.find( "\r\n", position + 1 );

                next = next_space < next_line ? next_space : next_line;
                if ( next_return_line < next )
                {
                    next = next_return_line;
                    skip = 2;
                }

                if ( next == string::npos )
                {
                    next = text.length();
                }

                const string arg = text.substr( position, next - position );
                if ( arg.length() > 0 and arg != "\n" and arg != "\r" and arg != "\r\n" )
                {
                    read_args.push_back( arg );
                }
                position = next + skip;
            }

            parse_arguments( read_args );
        }

        // Use help info of registered bindings to display usage information
        void print_bindings()
        {
            let& flag_bindings            = internal::get_flag_bindings();
            let& flag_parameters_bindings = internal::get_flag_parameters_bindings();

            auto& log = get_console_log();

            log.print( "\1 command line arguments found", flag_parameters_bindings.size() + flag_bindings.size() );

            // Print all parameterless bindings first (in nondeterministic order of binding)
            foreach ( pair : flag_bindings )
            {
                let& flag    = pair.first;
                let& binding = pair.second;

                if ( not binding.is_alt )
                {
                    // Flag name
                    get_console_log().print_additional( flag );

                    // Alternate
                    if ( binding.alt != internal::empty_alt_flag )
                    {
                        log << " (" << binding.alt << ")";
                    }

                    // Description
                    log.print_additional( "  \1", binding.description );
                }
            }

            // Print all bindings with parameters (in nondeterministic order of binding)
            foreach ( pair : flag_parameters_bindings )
            {
                let& flag    = pair.first;
                let& binding = pair.second;

                if ( not binding.is_alt )
                {
                    // Flag name
                    log.print_additional( "\1", flag );

                    // Alternate
                    if ( binding.alt != internal::empty_alt_flag )
                    {
                        log << " (" << binding.alt << ")";
                    }

                    // Parameters
                    for ( uint i : range( binding.parameter_count ) )
                    {
                        log << " < << binding.parameter_names[i] << >";
                    }

                    // Description
                    log.print_additional( "  \1", binding.description );
                }
            }
        }

        // Command bindings
        bind_console_parameters( "args-file", "af", "read command line arguments from a file", parse_arguments_from_file, "file path" );
        bind_console_flag( "help", "h", "show this help message", print_bindings );
    } // namespace console
} // namespace rnjin