/*  Part of rnjin
    (c) Rajin Shankar, 2019
        rajinshankar.com
*/

#include "log.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>


namespace rnjin
{
    namespace log
    {
        // Create a new output log
        source::source( const string& name, const write_to_file output_mode )
        {
            this->name    = name;
            output_stream = &std::cout;

            std::stringstream name_stream;
            auto current_time       = std::time( nullptr );
            auto current_local_time = *std::localtime( &current_time );
            name_stream << "logs/" << name << std::put_time( &current_local_time, "_%m%d%y_%H%M.log" );
            output_file_name = name_stream.str();

            output_file_mode = output_mode;
            if ( output_file_mode != write_to_file::never )
            {
                output_file_stream.open( output_file_name );
            }

            if ( output_file_mode == write_to_file::in_background )
            {
                // Start write to file thread
            }

            active_channel      = 0;
            channels["default"] = 0;

            print( "Log created" );
        }
        source::~source()
        {
            print( "Log ended" );
        }

        // Assign a number to a channel name
        void source::set_channel( const unsigned int number, const string& channel_name )
        {
            if ( channels.count( channel_name ) )
            {
                channels.erase( channel_name );
            }
            channels[channel_name] = number;
        }

        // Control which channels are enabled by a mask
        void source::enable_channels( const bitmask mask, const affect output_type )
        {
            if ( output_type & affect::console )
            {
                console_channel_mask |= mask;
            }
            if ( output_type & affect::file )
            {
                file_channel_mask |= mask;
            }
        }
        void source::disable_channels( const bitmask mask, const affect output_type )
        {
            if ( output_type & affect::console )
            {
                console_channel_mask /= mask;
            }
            if ( output_type & affect::file )
            {
                file_channel_mask /= mask;
            }
        }

        // Control which channels are enabled by number
        void source::enable_channel( const unsigned int channel_number, const affect output_type )
        {
            if ( bitmask::is_valid_bit( channel_number ) )
            {
                if ( output_type & affect::console )
                {
                    console_channel_mask += channel_number;
                }
                if ( output_type & affect::file )
                {
                    file_channel_mask += channel_number;
                }
            }
            else
            {
                printf( "Can't enable invalid channel #\1!", { s( channel_number ) }, bitmask::all() );
            }
        }
        void source::disable_channel( const unsigned int channel_number, const affect output_type )
        {
            if ( bitmask::is_valid_bit( channel_number ) )
            {
                if ( output_type & affect::console )
                {
                    console_channel_mask -= channel_number;
                }
                if ( output_type & affect::file )
                {
                    file_channel_mask -= channel_number;
                }
            }
            else
            {
                printf( "Can't disable invalid channel #\1!", { s( channel_number ) }, bitmask::all() );
            }
        }

        // Control which channels are enabled by name
        void source::enable_channel( const string& channel_name, const affect output_type )
        {
            if ( channels.count( channel_name ) )
            {
                enable_channel( channels[channel_name], output_type );
            }
            else
            {
                printf( "Can't enable nonexistent channel '\1'!", { channel_name }, bitmask::all() );
            }
        }
        void source::disable_channel( const string& channel_name, const affect output_type )
        {
            if ( channels.count( channel_name ) )
            {
                disable_channel( channels[channel_name], output_type );
            }
            else
            {
                printf( "Can't disable nonexistent channel '\1'!", { channel_name }, bitmask::all() );
            }
        }

        // Control what channel is written to using the << operator
        void source::set_active_channel( const unsigned int channel_number )
        {
            if ( bitmask::is_valid_bit( channel_number ) )
            {
                active_channel = channel_number;
            }
            else
            {
                printf( "Can't set invalid channel #\1 active!", { s( channel_number ) }, bitmask::all() );
            }
        }
        void source::set_active_channel( const string& channel_name )
        {
            if ( channels.count( channel_name ) )
            {
                set_active_channel( channels[channel_name] );
            }
            else
            {
                printf( "Can't set nonexistent channel '\1' active!", { channel_name }, bitmask::all() );
            }
        }

        // Public printing API
        source& source::print( const string& message, const bitmask channels )
        {
            if ( console_channel_mask && channels )
            {
                _write( message );
            }
            if ( file_channel_mask && channels )
            {
                if ( output_file_mode == write_to_file::immediately )
                {
                    _store( message );
                }
                else if ( output_file_mode == write_to_file::in_background )
                {
                    _queue( message );
                }
            }

            return *this;
        }
        source& source::print( const string& message, const unsigned int channel_number )
        {
            if ( !bitmask::is_valid_bit( channel_number ) )
            {
                printf( "Can't print to invalid channel #\1!", { s( channel_number ) }, bitmask::all() );
                return *this;
            }

            if ( console_channel_mask[channel_number] )
            {
                _write( message );
            }
            if ( file_channel_mask[channel_number] )
            {
                if ( output_file_mode == write_to_file::immediately )
                {
                    _store( message );
                }
                else if ( output_file_mode == write_to_file::in_background )
                {
                    _queue( message );
                }
            }

            return *this;
        }
        source& source::print( const string& message, const string& channel_name )
        {
            if ( channels.count( channel_name ) )
            {
                print( message, channels[channel_name] );
            }
            else
            {
                printf( "Can't print to nonexistent channel '\1'!", { channel_name }, bitmask::all() );
            }
            return *this;
        }

        source& source::printf( const string& message, const list<string> args, const bitmask channels )
        {
            if ( console_channel_mask && channels )
            {
                _writef( message, args );
            }
            if ( file_channel_mask && channels )
            {
                if ( output_file_mode == write_to_file::immediately )
                {
                    _storef( message, args );
                }
                else if ( output_file_mode == write_to_file::in_background )
                {
                    _queuef( message, args );
                }
            }

            return *this;
        }
        source& source::printf( const string& message, const list<string> args, const unsigned int channel_number )
        {
            if ( !bitmask::is_valid_bit( channel_number ) )
            {
                printf( "Can't print to invalid channel #\1!", { s( channel_number ) }, bitmask::all() );
                return *this;
            }

            if ( console_channel_mask[channel_number] )
            {
                _writef( message, args );
            }
            if ( file_channel_mask[channel_number] )
            {
                if ( output_file_mode == write_to_file::immediately )
                {
                    _storef( message, args );
                }
                else if ( output_file_mode == write_to_file::in_background )
                {
                    _queuef( message, args );
                }
            }

            return *this;
        }
        source& source::printf( const string& message, const list<string> args, const string& channel_name )
        {
            if ( channels.count( channel_name ) )
            {
                printf( message, args, channels[channel_name] );
            }
            else
            {
                printf( "Can't print to nonexistent channel '\1'!", { channel_name }, bitmask::all() );
            }
            return *this;
        }

        source& source::operator<<( const string& message )
        {
            if ( console_channel_mask[active_channel] )
            {
                _write( message, false );
            }
            if ( file_channel_mask[active_channel] )
            {
                if ( output_file_mode == write_to_file::immediately )
                {
                    _store( message, false );
                }
                else if ( output_file_mode == write_to_file::in_background )
                {
                    _queue( message, false );
                }
            }

            return *this;
        }
        source& source::operator<<( const log::start output_name )
        {
            *this << name << ": ";
            return *this;
        }
        source& source::operator<<( const log::line output_name )
        {
            *this << "\n";
            return *this;
        }
        source& source::operator<<( const log::use_channel output_channel )
        {
            if (output_channel.use_name)
            {
                set_active_channel(output_channel.name);
            }
            else
            {
                set_active_channel(output_channel.number);
            }
            
            return *this;
        }

        // Basic write and store operations
        void source::_write( const string& message, const bool line )
        {
            if ( line )
            {
                ( *output_stream ) << name << ": ";
            }

            ( *output_stream ) << message;

            if ( line )
            {
                ( *output_stream ) << std::endl;
            }
        }
        void source::_store( const string& message, const bool line )
        {
            if ( output_file_stream.good() )
            {
                if ( line )
                {
                    output_file_stream << name << ": ";
                }

                output_file_stream << message;

                if ( line )
                {
                    output_file_stream << std::endl;
                }
            }
        }

        // Write and store with formatting
        void source::_writef( const string& message, const list<string> args, const bool line )
        {
            if ( line )
            {
                ( *output_stream ) << name << ": ";
            }
            for ( int i = 0; i < message.length(); i++ )
            {
                unsigned char c = message[i];
                if ( c == 0x0 )
                {
                    break;
                }
                else if ( c <= args.size() )
                {
                    ( *output_stream ) << args[c - 1];
                }
                else
                {
                    ( *output_stream ) << c;
                }
            }
            if ( line )
            {
                ( *output_stream ) << std::endl;
            }
        }
        void source::_storef( const string& message, const list<string> args, const bool line )
        {
            if ( output_file_stream.good() )
            {
                if ( line )
                {
                    output_file_stream << name << ": ";
                }
                for ( int i = 0; i < message.length(); i++ )
                {
                    char c = message[i];
                    if ( c == 0x0 )
                    {
                        break;
                    }
                    else if ( c <= args.size() )
                    {
                        output_file_stream << args[c - 1];
                    }
                    else
                    {
                        output_file_stream << c;
                    }
                }
                if ( line )
                {
                    output_file_stream << std::endl;
                }
            }
        }

        void source::_queue( const string& message, const bool line ) {}
        void source::_queuef( const string& message, const list<string> args, const bool line ) {}

        // // Basic write and store, check any of multiple channels
        // void source::write( const string& message, const bitmask channels )
        // {
        //     if ( console_channel_mask && channels )
        //     {
        //         _write( message );
        //     }
        // }
        // void source::store( const string& message, const bitmask channels )
        // {
        //     if ( file_channel_mask && channels )
        //     {
        //         _store( message );
        //     }
        // }

        // // Basic write and store, check one channel by number
        // void source::write( const string& message, const unsigned int channel_number )
        // {
        //     if ( bitmask::is_valid_bit( channel_number ) )
        //     {
        //         if ( console_channel_mask[channel_number] )
        //         {
        //             _write( message );
        //         }
        //     }
        //     else
        //     {
        //         _writef( "Can't write to invalid channel #\1", { s( channel_number ) } );
        //     }
        // }
        // void source::store( const string& message, const unsigned int channel_number )
        // {
        //     if ( bitmask::is_valid_bit( channel_number ) )
        //     {
        //         if ( file_channel_mask[channel_number] )
        //         {
        //             _store( message );
        //         }
        //     }
        //     else
        //     {
        //         _writef( "Can't store to invalid channel #\1", { s( channel_number ) } );
        //     }
        // }

        // // Basic write and store, check one channel by name
        // void source::write( const string& message, const string& channel_name )
        // {
        //     if ( channels.count( channel_name ) )
        //     {
        //         write( message, channels[channel_name] );
        //     }
        //     else
        //     {
        //         _writef( "Can't write to invalid channel '\1'", { channel_name } );
        //     }
        // }
        // void source::store( const string& message, const string& channel_name )
        // {
        //     if ( channels.count( channel_name ) )
        //     {
        //         store( message, channels[channel_name] );
        //     }
        //     else
        //     {
        //         _storef( "Can't store to invalid channel '\1'", { channel_name } );
        //     }
        // }

        // // Write and store with format, check any of multiple channels
        // void source::writef( const string& message, const list<string> args, const bitmask channels )
        // {
        //     if ( console_channel_mask && channels )
        //     {
        //         _writef( message, args );
        //     }
        // }
        // void source::storef( const string& message, const list<string> args, const bitmask channels )
        // {
        //     if ( file_channel_mask && channels )
        //     {
        //         _storef( message, args );
        //     }
        // }

        // // Write and store with format, check one channel by number
        // void source::writef( const string& message, const list<string> args, const unsigned int channel_number )
        // {
        //     if ( bitmask::is_valid_bit( channel_number ) )
        //     {
        //         if ( console_channel_mask[channel_number] )
        //         {
        //             _writef( message, args );
        //         }
        //     }
        //     else
        //     {
        //         _writef( "Can't write to invalid channel #\1", { s( channel_number ) } );
        //     }
        // }
        // void source::storef( const string& message, const list<string> args, const unsigned int channel_number )
        // {
        //     if ( bitmask::is_valid_bit( channel_number ) )
        //     {
        //         if ( file_channel_mask[channel_number] )
        //         {
        //             _storef( message, args );
        //         }
        //     }
        //     else
        //     {
        //         _writef( "Can't store to invalid channel #\1", { s( channel_number ) } );
        //     }
        // }

        // // Write and store with format, check one channel by name
        // void source::writef( const string& message, const list<string> args, const string& channel_name )
        // {
        //     if ( channels.count( channel_name ) )
        //     {
        //         writef( message, args, channels[channel_name] );
        //     }
        //     else
        //     {
        //         _writef( "Can't write to invalid channel '\1'", { channel_name } );
        //     }
        // }
        // void source::storef( const string& message, const list<string> args, const string& channel_name )
        // {
        //     if ( channels.count( channel_name ) )
        //     {
        //         storef( message, args, channels[channel_name] );
        //     }
        //     else
        //     {
        //         _storef( "Can't store to invalid channel '\1'", { channel_name } );
        //     }
        // }
    } // namespace log
} // namespace rnjin