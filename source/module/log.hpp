#include "core.hpp"
#include <iostream>
#include <fstream>

namespace rnjin::log
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

    class output_log
    {

        public:
        output_log( const string& name, const write_to_file output_mode, const string& file_path );
        ~output_log();

        void set_channel( const unsigned int number, const string& channel_name );

        void enable_channel( const string& channel_name, const affect output_type = affect::both );
        void disable_channel( const string& channel_name, const affect output_type = affect::both );

        void enable_channel( const unsigned int channel_number, const affect output_type = affect::both );
        void disable_channel( const unsigned int channel_number, const affect output_type = affect::both );

        void enable_channels( const bitmask mask, const affect output_type = affect::both );
        void disable_channels( const bitmask mask, const affect output_type = affect::both );

        output_log* const operator<<( const string& message );
        output_log* const print( const string& message, const bitmask channels );
        output_log* const print( const string& message, const unsigned int channel_number = 0 );
        output_log* const print( const string& message, const string& channel_name );
        
        output_log* const printf( const string& message, const list<string> args, const bitmask channels);
        output_log* const printf( const string& message, const list<string> args, const unsigned int channel_number = 0 );
        output_log* const printf( const string& message, const list<string> args, const string& channel_name );

        void set_active_channel(const unsigned int channel_number);
        void set_active_channel(const string& channel_name);

        private:
        // Log config
        string name;

        // Console output config
        std::ostream* output_stream;

        // Output file config
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
} // namespace rnjin::log