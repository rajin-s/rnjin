/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "containers.hpp"

#include "macro.hpp"

namespace rnjin
{
    const string string_from_c_strings( const char** c_strings, uint count )
    {
        // Get the final string length as the sum of all the substrings
        size_t total_length = 0;
        for ( uint i = 0; i < count; i++ )
        {
            const char* c_string = c_strings[i];
            total_length += strlen( c_string );
        }

        // Set the size of the final string buffer
        string result;
        result.resize( total_length );

        // Copy each of the strings into the result
        uint result_index = 0;
        for ( uint i = 0; i < count; i++ )
        {
            const char* c_string = c_strings[i];
            for ( const char* c = c_string; c and *c; c += sizeof( char ) )
            {
                result[result_index] = *c;
                result_index++;
            }
        }

        return result;
    }

    range& range::begin()
    {
        return *this;
    }
    range& range::end()
    {
        return *this;
    }
    const uint range::operator*()
    {
        return start_i;
    }
    const bool range::operator!=( const range& other )
    {
        return start_i < end_i;
    }
    range& range::operator++()
    {
        start_i++;
        return *this;
    }
} // namespace rnjin