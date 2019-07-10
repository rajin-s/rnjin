/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "debug.hpp"

#include "macro.hpp"
#include "platform.hpp"

namespace rnjin
{
    namespace debug
    {
        const string get_call_string( const char* file_path, const char* function_name, const char* line_number )
        {
            // Find the last directory separator (if it exists)
            const char* file_name = strrchr( file_path, platform::directory_separator );
            if ( file_name )
            {
                // Skip one character (the last separator)
                file_name += sizeof( char );
            }
            else
            {
                file_name = file_path;
            }

            // Combine parts
            const char* parts[]      = { "(", file_name, ")<", function_name, ":", line_number, ">" };
            const string call_string = string_from_c_strings( parts, 7 );

            return call_string;
        }
    } // namespace debug
} // namespace rnjin