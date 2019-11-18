/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include standard_library( fstream )

#include rnjin_module( core )
#include rnjin_module( log )

namespace rnjin
{
    namespace io
    {
        extern log::source::masked file_log_verbose;
        extern log::source::masked file_log_errors;

        class file
        {
            using file_stream = std::fstream;

            public: // enums
            enum class mode
            {
                read       = bit( 0 ),
                write      = bit( 1 ),
                read_write = bits( 0, 1 )
            };

            public: // methods
            file( const string& path, const mode file_mode );
            file();
            ~file();

            void open( const string& path, const mode file_mode );
            void close();

            // Check that the file was opened properly
            const bool is_valid() const;

            // Move to a specific position
            void seek( const uint position );

            // Move forward without reading/writing
            void skip( const uint bytes );

            template <typename T>
            void skip_var()
            {
                skip( sizeof( T ) );
            }
            template <>
            void skip_var<string>();

            // Move backward without reading/writing
            void reverse( const uint bytes );

            // Write some value to the file
            template <typename T>
            void write_var( const T& value )
            {
                check_error_condition( return, file_log_errors, not is_valid(), "Can't write to invalid file '\1'", path );
                check_error_condition( return, file_log_errors, not file_mode.contains( (uint) mode::write ), "File '\1' not opened for writing", path );

                let var_size      = sizeof( T );
                let value_pointer = (byte*) &value;

                write_bytes( var_size, var_size, value_pointer );
            }
            template <>
            void write_var<string>( const string& value );
            void write_string( const string& value );
            void write_string( const char* value );

            // Read some value from the file
            template <typename T>
            const T read_var()
            {
                T value{};

                check_error_condition( return value, file_log_errors, not is_valid(), "Can't read from invalid file '\1'", path );
                check_error_condition( return value, file_log_errors, not file_mode.contains( (uint) mode::read ), "File '\1' not opened for reading", path );

                let var_size      = sizeof( T );
                let value_pointer = (byte*) &value;

                read_bytes( var_size, var_size, value_pointer );

                return value;
            }
            template <>
            const string read_var<string>();
            const string read_string();

            // Write multiple values to the file
            template <typename T>
            void write_buffer( const list<T>& values )
            {
                check_error_condition( return, file_log_errors, not is_valid(), "Can't write buffer to invalid file '\1'", path );
                check_error_condition( return, file_log_errors, not file_mode.contains( (uint) mode::write ), "File '\1' not opened for writing", path );

                const uint buffer_length   = values.size();
                const uint element_size    = sizeof( T );
                const uint buffer_size     = element_size * buffer_length;
                const byte* buffer_pointer = (byte*) values.data();

                write_var( buffer_length );
                write_bytes( buffer_size, element_size, buffer_pointer );
            }
            template <>
            void write_buffer<string>( const list<string>& values );

            // Read multiple values from the file
            template <typename T>
            list<T> read_buffer()
            {
                list<T> values;

                check_error_condition( return values, file_log_errors, not is_valid(), "Can't read buffer from invalid file '\1'", path );
                check_error_condition( return values, file_log_errors, not file_mode.contains( (uint) mode::read ), "File '\1' not opened for reading", path );

                const uint element_size  = sizeof( T );
                const uint buffer_length = read_var<uint>();

                const uint buffer_size = element_size * buffer_length;
                values.resize( buffer_length );

                nonconst byte* buffer_pointer = (byte*) values.data();

                read_bytes( buffer_size, element_size, buffer_pointer );

                return values;
            }
            template <>
            list<string> read_buffer<string>();

            void write_all_text( const string& text );
            string read_all_text();

            private: // methods
            void open();
            void write_bytes( const uint count, const uint stride, const byte* source );
            void read_bytes( const uint count, const uint stride, nonconst byte* destination );

            private: // members
            bool valid;
            string path;
            bitmask file_mode;
            uint size;
            file_stream* stream;

            public: // static methods
            static string read_text_from( const string& path );
        };
    } // namespace io
} // namespace rnjin