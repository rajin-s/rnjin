/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "file.hpp"

namespace rnjin
{
    namespace io
    {
        enum class log_flag
        {
            verbose = 2,
            errors  = 3
        };

        static log::source& get_file_log()
        {
            static log::source file_log(
                "rnjin.file", log::output_mode::immediately, log::output_mode::immediately,
                {
                    { "verbose", (uint) log_flag::verbose, false },
                    { "errors", (uint) log_flag::errors, true },
                } //
            );
            return file_log;
        }

        log::source::masked file_log_verbose        = get_file_log().mask( log_flag::verbose );
        log::source::masked file_log_errors         = get_file_log().mask( log_flag::errors );
        log::source::masked file_log_verbose_errors = get_file_log().mask( log_flag::verbose, log_flag::errors );

        // Create a file and open it
        file::file( const string& path, const mode _file_mode ) : path( path ), file_mode( (uint) _file_mode )
        {
            open();
        }

        // Create a file but don't open it
        file::file() : valid( false ) {}

        // Open a file given a path and mode
        void file::open( const string& path, const mode _file_mode )
        {
            file_mode  = bitmask( (uint) _file_mode );
            this->path = path;

            open();
        }

        // Open the file with its current path and mode
        void file::open()
        {
            check_error_condition( return, file_log_errors, path.empty(), "Can't open file without path" );
            check_error_condition( return, file_log_errors, file_mode == no_bits, "Can't open file without mode" );

            // Open files at the end by default to easily get the size
            file_stream::openmode open_mode = file_stream::ate | file_stream::binary;

            // Convert file::mode to fstream::openmode
            if ( file_mode.contains( (uint) mode::read ) )
            {
                open_mode |= file_stream::in;
            }
            if ( file_mode.contains( (uint) mode::write ) )
            {
                open_mode |= file_stream::out;
            }

            valid = false;

            // allocate a new fstream
            stream = new file_stream;
            check_error_condition( return, file_log_errors, stream == nullptr, "Failed to create file stream for file '\1'", path );

            // open the file
            stream->open( this->path, open_mode );
            check_error_condition( return, file_log_errors, not stream->good(), "Failed to open file '\1'", path );

            // get the file size and reset back to the beginning of the file
            size = stream->tellg();
            stream->seekg( 0 );

            valid = true;
        }

        // Close the file and free resources
        void file::close()
        {
            valid = false;
            if ( stream != nullptr )
            {
                if ( stream->is_open() )
                {
                    stream->close();
                }
                delete stream;
                stream = nullptr;
            }
        }

        // Close the file on destruction
        file::~file()
        {
            close();
        }

        // Check that the file was opened properly
        const bool file::is_valid() const
        {
            check_error_condition( pass, file_log_verbose_errors, not valid, "" );
            check_error_condition( pass, file_log_verbose_errors, stream == nullptr, "" );
            check_error_condition( pass, file_log_verbose_errors, stream->bad(), "" );
            check_error_condition( pass, file_log_verbose_errors, stream->eof(), "" );
            check_error_condition( pass, file_log_verbose_errors, stream->fail(), "" );

            return valid and stream != nullptr and stream->good();
        }

        // Move to a specific position
        void file::seek( const uint position )
        {
            check_error_condition( return, file_log_errors, not is_valid(), "Can't seek in invalid file '\1'", path );
            stream->seekg( position );
        }

        // Move forward without reading/writing
        void file::skip( const uint bytes )
        {
            check_error_condition( return, file_log_errors, not is_valid(), "Can't skip in invalid file '\1'", path );
            stream->seekg( bytes, file_stream::right );
        }

        // Move backward without reading/writing
        void file::reverse( const uint bytes )
        {
            check_error_condition( return, file_log_errors, not is_valid(), "Can't reverse in invalid file '\1'", path );
            stream->seekg( bytes, file_stream::left );
        }

        // Write some string value to the file
        template <>
        void file::write_var<string>( const string& value )
        {
            check_error_condition( return, file_log_errors, not is_valid(), "Can't write to invalid file '\1'", path );
            check_error_condition( return, file_log_errors, not file_mode.contains( (uint) mode::write ), "File '\1' not opened for writing", path );

            const uint string_size     = value.size();
            const byte* buffer_pointer = (const byte*) value.c_str();

            write_var( string_size );
            write_bytes( string_size, buffer_pointer );
        }
        void file::write_string( const string& value )
        {
            write_var( value );
        }
        void file::write_string( const char* value )
        {
            write_var<string>( value );
        }

        // Read some string value from the file
        template <>
        const string file::read_var<string>()
        {
            string value;

            check_error_condition( return value, file_log_errors, not is_valid(), "Can't read from invalid file '\1'", path );
            check_error_condition( return value, file_log_errors, not file_mode.contains( (uint) mode::read ), "File '\1' not opened for reading", path );

            let string_size = read_var<uint>();
            value.resize( string_size );

            let buffer_pointer = (byte*) value.data();
            read_bytes( string_size, buffer_pointer );

            return value;
        }
        const string file::read_string()
        {
            return read_var<string>();
        }

        // Write multiple string values to the file
        template <>
        void file::write_buffer<string>( const list<string>& values )
        {
            check_error_condition( return, file_log_errors, not is_valid(), "Can't write buffer to invalid file '\1'", path );
            check_error_condition( return, file_log_errors, not file_mode.contains( (uint) mode::write ), "File '\1' not opened for writing", path );

            const uint buffer_length = values.size();

            write_var( buffer_length );
            foreach ( value : values )
            {
                write_var( value );
            }
        }

        // Read multiple string values from the file
        template <>
        list<string> file::read_buffer<string>()
        {
            list<string> values;

            check_error_condition( return values, file_log_errors, not is_valid(), "Can't read buffer from invalid file '\1'", path );
            check_error_condition( return values, file_log_errors, not file_mode.contains( (uint) mode::read ), "File '\1' not opened for reading", path );

            const uint buffer_length = read_var<uint>();
            values.resize( buffer_length );

            for ( uint i : range( buffer_length ) )
            {
                values[i] = read_var<string>();
            }

            return values;
        }

        // Write bytes from an arbitrary data buffer
        // note: called from higher level read/write that check size, etc.
        void file::write_bytes( const uint count, const byte* source )
        {
            if ( count == 0 ) return;

            auto out = (std::ostream*) stream;
            out->write( (const char*) source, count );
        }

        // Read bytes into an arbitrary data buffer
        // note: called from higher level read/write that check size, etc.
        void file::read_bytes( const uint count, nonconst byte* destination )
        {
            if ( count == 0 ) return;

            auto in = (std::istream*) stream;
            in->read( (char*) destination, count );
        }

        // Write a text file
        void file::write_all_text( const string& text )
        {
            let string_size    = text.size() * sizeof( char );
            let buffer_pointer = (const byte*) text.c_str();
            write_bytes( string_size, buffer_pointer );

            file_log_verbose.print( "Write \1B of text to '\2'", string_size, path );
        }

        // Read a text file
        string file::read_all_text()
        {
            string text;
            text.resize( size / sizeof( char ) );

            let buffer_pointer = (nonconst byte*) text.c_str();
            read_bytes( size, buffer_pointer );

            file_log_verbose.print( "Read \1B of text from '\2'", size, path );
            return text;
        }

        // We need these template specializations to be instantiated here, since having them elsewhere won't trigger it
        // This function otherwise does nothing
        void __()
        {
            file f( "__", file::mode::write );
            f.read_var<string>();
            f.write_var<string>( "" );
            f.read_buffer<string>();
            f.write_buffer<string>( { "" } );
        }
    } // namespace io
} // namespace rnjin