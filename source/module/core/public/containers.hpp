/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

// STL data structures
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Aliases for STL types
namespace rnjin
{
    // numeric types
    using byte = uint8_t;
    using uint = uint32_t;

    using uint64 = uint64_t;
    using uint16 = uint16_t;

    // strings
    using string = std::string;
    const string string_from_c_strings( const char** c_strings, uint count );

    // containers
    template <typename T>
    using list = std::vector<T>;

    template <typename K, typename V>
    using dictionary = std::unordered_map<K, V>;

    template <typename T>
    using set = std::unordered_set<T>;

    // range for python-style for( uint i : range(0, 10) )
    // note: only supports 32-bit unsigned values
    class range
    {
        public:
        range( const uint first, const uint limit ) : start_i( first ), end_i( limit ) {}
        range( const uint limit ) : start_i( 0 ), end_i( limit ) {}

        range& begin();
        range& end();
        const uint operator*();
        const bool operator!=( const range& other );
        range& operator++();

        private:
        uint start_i;
        const uint end_i;
    };

    template <typename T>
    class child_class
    {
        public: // methods
        child_class( T& parent ) : parent( parent ) {}

        void set_parent( T& new_parent )
        {
            parent = new_parent;
        }

        protected: // members
        T& parent;
    };

    // Simple function pointers
    using action = void ( * )( void );
} // namespace rnjin