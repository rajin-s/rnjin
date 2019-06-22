/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

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

#pragma once

#include "core.hpp"
#include <fstream>
#include <iostream>

using namespace rnjin::core;

namespace rnjin
{
    namespace log
    {
        // How should a log source handle writing to disk?
        // in_background: Write queued messages to a log file on a dedicated thread at regular intervals
        // immediately: Do file writing at the same time as console output
        // never: Only log to console, not a file on disk
        enum write_to_file
        {
            in_background,
            immediately,
            never
        };

        // How should log source mask operations affect output to the console and files?
        // console: Affect the mask of only console output
        // file: Affect the mask of only file output
        // both: Affect the masks of both console and file output
        enum affect
        {
            console = 1,
            file    = 2,
            both    = 3
        };

        // Stream-style print modifiers
        //  note: maybe there's a better way of handling these...

        // Stream-style token to print the standard log starting message ("<name>: ")
        struct start
        {
        };

        //...
        struct use_channel
        {
            use_channel( const unsigned int number ) : number( number ), name( "" ), use_name( false ) {}
            use_channel( const string& name ) : name( name ), number( -1 ), use_name( true ) {}

            const unsigned int number;
            const string& name;
            const bool use_name;
        };

        //...
        struct line
        {
        };

        //...
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

            void write( const string& message, const bool line = true );
            void store( const string& message, const bool line = true );
            void queue_store( const string& message, const bool line = true );

            void writef( const string& message, const list<string> args, const bool line = true );
            void storef( const string& message, const list<string> args, const bool line = true );
            void queue_storef( const string& message, const list<string> args, const bool line = true );
        };

        extern source main;
    } // namespace log
} // namespace rnjin