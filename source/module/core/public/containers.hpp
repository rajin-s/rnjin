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
    using string = std::string;

    template <typename T>
    using list = std::vector<T>;

    template <typename K, typename V>
    using dictionary = std::unordered_map<K, V>;

    template <typename T>
    using set = std::unordered_set<T>;

    using uint = uint32_t;
    using byte = uint8_t;

    class range
    {
        public:
        range( const uint first, const uint limit ) : start_i( first ), end_i( limit ) {}

        range& begin()
        {
            return *this;
        }
        range& end()
        {
            return *this;
        }

        const uint operator*()
        {
            return start_i;
        }

        const bool operator!=( const range& other )
        {
            return start_i < end_i;
        }

        range& operator++()
        {
            start_i++;
            return *this;
        }

        private:
        uint start_i;
        const uint end_i;
    };
} // namespace rnjin