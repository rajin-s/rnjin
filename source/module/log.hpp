/*  Part of rnjin
    (c) Rajin Shankar, 2019
        rajinshankar.com
*/

/*  Logging API
    Supports arbitrary creation of output logs, each writing to their own file and the console
        note: currently not thread-safe, feature will be added soon!

    Supports channel enable / disable for up to 64 channels (32 for 32-bit architecture...)
        Channels can be named, and messages can be passed through any number of channels
        Example: Engine log has channels:
            0 -> default
            1 -> warning info
            2 -> error info
            3 -> verbose output

    Supports print formatting for up to 7 arguments using \1 - \7 characters
        Using any more than 9 (\11) will lead to unexpected behavior

    Supports << operator like cout (calls will not create line breaks automatically)
        Use set_active_channel to determine what channel subsequent << calls will write to
 */

#include "core.hpp"
#include <fstream>
#include <iostream>

namespace rnjin
{
    namespace log
    {
        enum write_to_file
        {
            in_background,
            immediately,
            never
        };
        enum affect
        {
            console = 1,
            file    = 2,
            both    = 3
        };

        // Stream-style print modifiers
        struct start
        {
        };
        struct use_channel
        {
            use_channel( const unsigned int number ) : number( number ), name( nullptr ), use_name( false ) {}
            use_channel( const string& name ) : name( name ), number( -1 ), use_name( true ) {}

            const unsigned int number;
            const string& name;
            const bool use_name;
        };
        struct line
        {
        };

        class source
        {

            public:
            source( const string& name, const write_to_file output_mode );
            ~source();

            void set_channel( const unsigned int number, const string& channel_name );

            void enable_channel( const string& channel_name, const affect output_type = affect::both );
            void disable_channel( const string& channel_name, const affect output_type = affect::both );

            void enable_channel( const unsigned int channel_number, const affect output_type = affect::both );
            void disable_channel( const unsigned int channel_number, const affect output_type = affect::both );

            void enable_channels( const bitmask mask, const affect output_type = affect::both );
            void disable_channels( const bitmask mask, const affect output_type = affect::both );

            source& operator<<( const string& message );
            source& operator<<( const log::start output_name );
            source& operator<<( const log::use_channel output_channel );
            source& operator<<( const log::line output_line );

            source& print( const string& message, const bitmask channels );
            source& print( const string& message, const string& channel_name );
            source& print( const string& message, const unsigned int channel_number = 0 );

            source& printf( const string& message, const list<string> args, const bitmask channels );
            source& printf( const string& message, const list<string> args, const unsigned int channel_number = 0 );
            source& printf( const string& message, const list<string> args, const string& channel_name );

            void set_active_channel( const unsigned int channel_number );
            void set_active_channel( const string& channel_name );

            private:
            // Log config
            string name;

            // Console output config
            std::ostream* output_stream;

            // Output file config
            string output_file_name;
            std::ofstream output_file_stream;
            write_to_file output_file_mode;

            // Channel config
            bitmask console_channel_mask;
            bitmask file_channel_mask;
            dictionary<string, unsigned int> channels;
            unsigned int active_channel;

            void _write( const string& message, const bool line = true );
            void _store( const string& message, const bool line = true );
            void _queue( const string& message, const bool line = true );

            void _writef( const string& message, const list<string> args, const bool line = true );
            void _storef( const string& message, const list<string> args, const bool line = true );
            void _queuef( const string& message, const list<string> args, const bool line = true );

            // void write( const string& message, const bitmask channels );
            // void store( const string& message, const bitmask channels );
            // void write( const string& message, const unsigned int channel_number = 0 );
            // void store( const string& message, const unsigned int channel_number = 0 );
            // void write( const string& message, const string& channel_name );
            // void store( const string& message, const string& channel_name );

            // void writef( const string& message, const list<string> args, const unsigned int channel_number = 0 );
            // void storef( const string& message, const list<string> args, const unsigned int channel_number = 0 );
            // void writef( const string& message, const list<string> args, const string& channel_name );
            // void storef( const string& message, const list<string> args, const string& channel_name );
            // void writef( const string& message, const list<string> args, const bitmask channels );
            // void storef( const string& message, const list<string> args, const bitmask channels );
        };
    } // namespace log
} // namespace rnjin