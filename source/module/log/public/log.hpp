/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include standard_library( fstream )
#include standard_library( iostream )
#include standard_library( ostream )

#include rnjin_module( core )

using namespace rnjin::core;

namespace rnjin
{
    namespace log
    {
        class source;

        // Log icons for different print_* calls
        namespace icon
        {
            extern const string default;
            extern const string warning;
            extern const string error;
            extern const uint size;
        } // namespace icon

        // Utility class for tracking scope entry / exit
        template <typename log_type>
        class scope_tracker
        {
            public:
            scope_tracker( log_type& target, const char* scope_name ) : target( target ), scope_name( scope_name ), single_iter_finished( false )
            {
                target.print( "Begin \1", scope_name );
            }
            ~scope_tracker()
            {
                target.print_additional( "Finish \1", scope_name );
            }

            bool do_single_iter()
            {
                if ( single_iter_finished )
                {
                    return false;
                }
                else
                {
                    single_iter_finished = true;
                    return true;
                }
            }

            private:
            log_type& target;
            const char* scope_name;

            bool single_iter_finished;
        };

        /* clang-format off */
        #define tracked_subregion( target_log, name ) for ( rnjin::log::scope_tracker __( target_log, name ); __.do_single_iter(); )
        /* clang-format on */

        // How should the source interact with output destinations?
        // <never> don't write to console / any output files
        // <immediately> write to console / output files as soon as a message is written
        // <in_background> write to console / output files on a background thread (not implemented yet)
        enum class output_mode
        {
            never,
            immediately,
            in_background,
        };

        enum class begin
        {
            message,
            additional,
            warning,
            error,
            raw_line,
        };

        const string& get_log_directory();
        const string& get_log_extension();

        /* clang-format off */

        // Debug logging utilities
        // Check an error condition and output to the given log if it is met, recording the location
        // Action can be pass, return, continue, break, etc. to handle execution flow

        #define check_error_condition( action, target_log, condition, message_format, ... )                                                                           \
            if ( condition )                                                                                                                                          \
            {                                                                                                                                                         \
                target_log.print_error( message_format, __VA_ARGS__ );                                                                                                \
                target_log.print_additional( "'" #condition "' @ \1", rnjin::debug::get_call_string( __raw_file_string, __raw_function_string, __raw_line_string ) ); \
                action;                                                                                                                                               \
            }

        #define debug_checkpoint( target_log ) target_log.print( "Reach \1", rnjin::debug::get_call_string( __raw_file_string, __raw_function_string, __raw_line_string ) )

        /* clang-format on */

        // A single log source, capable of writing to any C++ ostream
        // Constructor can set up default output file and stdout, or call add_output
        class source
        {
            public: // constants
            static constexpr uint invalid_flag = ~0;

            public: // constructors
            source( const string& log_name, const output_mode console_output_mode, const output_mode file_output_mode );

            struct flag_info
            {
                const string& name;
                const uint number;
                const bool enabled;
            };
            source( const string& log_name, const output_mode console_output_mode, const output_mode file_output_mode, const list<flag_info> flags );

            ~source();

            public: // management
            void add_output( std::ostream& stream, const output_mode mode );

            void enable_flag( const uint number );
            void disable_flag( const uint number );

            void name_flag( const uint number, const string& name );
            const uint get_flag_number( const string& name ) const;

            public: // utility methods
            scope_tracker<source> track_scope( const char* scope_name )
            {
                return scope_tracker( *this, scope_name );
            }

            public: // general methods (no flags)
            // Output a formatted message. Supports up to 7 arguments (\1 - \7)
            template <typename... Ts>
            void print( const string& format, Ts... args )
            {
                write( begin::message );
                printf( format, args... );
            }

            // Output a formatted message addendum. Supports up to 7 arguments (\1 - \7)
            template <typename... Ts>
            void print_additional( const string& format, Ts... args )
            {
                write( begin::additional );
                printf( format, args... );
            }

            // Output a formatted warning message. Supports up to 7 arguments (\1 - \7)
            template <typename... Ts>
            void print_warning( const string& format, Ts... args )
            {
                write( begin::warning );
                printf( format, args... );
            }

            // Output a formatted error message. Supports up to 7 arguments (\1 - \7)
            template <typename... Ts>
            void print_error( const string& format, Ts... args )
            {
                write( begin::error );
                printf( format, args... );
            }

            // Stream-like write operator that can be chained.
            // Supports special log tokens begin::message, etc.
            template <typename T>
            source& operator<<( const T& value )
            {
                write( value );
                return *this;
            }

            public: // masked source
            class masked
            {
                public: // methods
                masked( source& target, const bitmask mask ) : target( target ), mask( mask ){};

                template <typename... Ts>
                void print( const string& format, Ts... args )
                {
                    if ( target.flag_output_mask.contains( mask ) )
                    {
                        target.print( format, args... );
                    }
                }
                template <typename... Ts>
                void print_additional( const string& format, Ts... args )
                {
                    if ( target.flag_output_mask.contains( mask ) )
                    {
                        target.print_additional( format, args... );
                    }
                }
                template <typename... Ts>
                void print_warning( const string& format, Ts... args )
                {
                    if ( target.flag_output_mask.contains( mask ) )
                    {
                        target.print_warning( format, args... );
                    }
                }
                template <typename... Ts>
                void print_error( const string& format, Ts... args )
                {
                    if ( target.flag_output_mask.contains( mask ) )
                    {
                        target.print_error( format, args... );
                    }
                }
                template <typename T>
                masked& operator<<( const T& value )
                {
                    if ( target.flag_output_mask.contains( mask ) )
                    {
                        target.operator<<( value );
                    }
                    return *this;
                }

                scope_tracker<masked> track_scope( const char* scope_name )
                {
                    return scope_tracker( *this, scope_name );
                }

                private: // members
                source& target;
                const bitmask mask;
            };

            template <typename... Ts>
            masked mask( Ts... flags )
            {
                masked masked_source( *this, bitmask( bits( flags... ) ) );
                return masked_source;
            }

            private: // members
            string name;
            string name_blank;

            string default_file_name;
            std::ofstream default_file_output_stream;

            struct output_target
            {
                std::ostream& stream;
                output_mode mode;
            };
            list<output_target> outputs;

            bitmask flag_output_mask;
            dictionary<string, uint> named_flags;

            private: // methods
            // Basic write function (forwards responsibilities to ostream << operator)
            template <typename T>
            void write( const T value )
            {
                foreach ( output : outputs )
                {
                    if ( output.mode == output_mode::immediately )
                    {
                        output.stream << value;
                    }
                }
            }

            // Write special log tokens to begin a message, etc.
            template <>
            void source::write( const begin value )
            {
                switch ( value )
                {
                    case begin::message:
                    {
                        print_prefix( icon::default, true );
                        break;
                    }
                    case begin::additional:
                    {
                        print_prefix( icon::default, false );
                        break;
                    }
                    case begin::warning:
                    {
                        print_prefix( icon::warning, true );
                        break;
                    }
                    case begin::error:
                    {
                        print_prefix( icon::error, true );
                        break;
                    }
                    case begin::raw_line:
                    {
                        write( "\n" );
                        break;
                    }
                    default:
                    {
                        print_prefix( icon::default, true );
                        break;
                    }
                }
            }

            // Called from print_* and source << begin::*
            void print_prefix( const string& icon, const bool show_name );

            private:
            // Helper functions for printf
            void write_range( const char** begin, uint* count, const char* reset );

#pragma region printf
            // Printf zone (only printf definitions beyond this point)
            // Each number of arguments needs to be defined as a specialization since arguments can appear an arbitrary number
            // of times at arbitrary positions (so no recursive 'consuming')

            template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            void printf( const string& format, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( const char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }
                        case '\2':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg2 );
                            break;
                        }
                        case '\3':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg3 );
                            break;
                        }
                        case '\4':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg4 );
                            break;
                        }
                        case '\5':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg5 );
                            break;
                        }
                        case '\6':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg6 );
                            break;
                        }
                        case '\7':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg7 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            void printf( const string& format, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }
                        case '\2':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg2 );
                            break;
                        }
                        case '\3':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg3 );
                            break;
                        }
                        case '\4':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg4 );
                            break;
                        }
                        case '\5':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg5 );
                            break;
                        }
                        case '\6':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg6 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            template <typename T1, typename T2, typename T3, typename T4, typename T5>
            void printf( const string& format, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }
                        case '\2':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg2 );
                            break;
                        }
                        case '\3':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg3 );
                            break;
                        }
                        case '\4':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg4 );
                            break;
                        }
                        case '\5':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg5 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            template <typename T1, typename T2, typename T3, typename T4>
            void printf( const string& format, T1 arg1, T2 arg2, T3 arg3, T4 arg4 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }
                        case '\2':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg2 );
                            break;
                        }
                        case '\3':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg3 );
                            break;
                        }
                        case '\4':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg4 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            template <typename T1, typename T2, typename T3>
            void printf( const string& format, T1 arg1, T2 arg2, T3 arg3 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }
                        case '\2':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg2 );
                            break;
                        }
                        case '\3':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg3 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            template <typename T1, typename T2>
            void printf( const string& format, T1 arg1, T2 arg2 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }
                        case '\2':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg2 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            template <typename T1>
            void printf( const string& format, T1 arg1 )
            {
                const char* read_begin = format.c_str();
                uint read_size         = 0;

                for ( const char* c = read_begin; c and *c; c += sizeof( char ) )
                {
                    switch ( *c )
                    {
                        case '\1':
                        {
                            write_range( &read_begin, &read_size, c );
                            write( arg1 );
                            break;
                        }

                        default:
                        {
                            read_size++;
                            break;
                        }
                    }
                }

                if ( read_size > 0 )
                {
                    write_range( &read_begin, &read_size, nullptr );
                }
            }

            void printf( const string& format )
            {
                write( format );
            }

#pragma endregion
        };

        extern source main;
        extern source::masked main_verbose;
        extern source::masked main_errors;
    } // namespace log
} // namespace rnjin