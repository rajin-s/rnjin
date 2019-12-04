/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

/*  Bitmask data structure with operator support:
        mask + number -> set a bit
        mask - number -> unset a bit
        mask & mask   -> bitwise and (intersection)
        mask | mask   -> bitwise or (union)
        mask / mask   -> bitwise nand (difference)
        ~mask         -> unary bitwise not
        mask[number]  -> whether a bit is set
        mask && mask  -> whether two masks have any common bits

    all/none methods to give fully set/unset masks
        note: by default, a mask is initialized to be all set

    Macros for computing a mask with one bit set/unset

    Uses bitmask::value_type to store value
        note: uint is word-sized, so different between 64 and 32-bit systems...
 */

#pragma once
#include <rnjin.hpp>

#include <ostream>

#include "macro.hpp"
#include "containers.hpp"

namespace rnjin
{
    namespace core
    {
        static constexpr uint bit( const uint n )
        {
            return 1u << n;
        }

        template <typename T, typename... Ts>
        static constexpr uint bits( const T first, const Ts... rest )
        {
            return bit( (uint) first ) | bits( rest... );
        }

        template <typename T>
        static constexpr uint bits( const T only )
        {
            return bit( (uint) only );
        }

        static constexpr uint all_bits = ~0u;
        static constexpr uint no_bits  = 0u;

        struct bitmask
        {
            typedef uint value_type;

            public:
            bitmask( value_type value ) : value( value ) {}
            bitmask() : value( ~0 ) {}

            // Is a given bit number valid in a bitmask?
            static bool is_valid_bit( const uint number );

            // (un)set a single bit
            bitmask operator+( const uint number ) const;
            bitmask operator-( const uint number ) const;
            bitmask operator+=( const uint number );
            bitmask operator-=( const uint number );

            // Combine two masks
            bitmask operator&( const bitmask other ) const;
            bitmask operator/( const bitmask other ) const;
            bitmask operator|( const bitmask other ) const;
            bitmask operator|=( const bitmask other );
            bitmask operator/=( const bitmask other );

            // Negate a mask
            bitmask operator~() const;

            // Check if a bit is set
            bool operator[]( const uint number ) const;

            // Check if all bits are set
            const bool contains( const bitmask other ) const;
            const bool contains( const uint value ) const;

            // Check if two masks share any bits
            bool operator&&( const bitmask other ) const;

            // Check if two masks are equal
            bool operator==( const bitmask other ) const;

            // Check if two masks are not equal
            bool operator!=( const bitmask other ) const;
            bool operator==( const uint other ) const;
            bool operator!=( const uint other ) const;

            bitmask clear();

            // All (un)set masks
            static bitmask all();
            static bitmask none();

            const char* toString( const char set, const char unset ) const;

            public: // accessors
            let raw_value get_value( value );

            private:
            value_type value;
        };

        std::ostream& operator<<( std::ostream& stream, const bitmask& mask );
    } // namespace core
} // namespace rnjin