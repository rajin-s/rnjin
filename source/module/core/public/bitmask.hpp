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
        note: unsigned int is word-sized, so different between 64 and 32-bit systems...
 */

#pragma once

#include <ostream>

#define bit( n ) ( 1 << n )
#define notbit( n ) ( ~( 1 << n ) )

namespace rnjin
{
    namespace core
    {
        struct bitmask;
        struct bitmask
        {
            typedef unsigned int value_type;

            public:
            bitmask() : value( ~0 ) {}
            bitmask( value_type value ) : value( value ) {}

            // Is a given bit number valid in a bitmask?
            static bool is_valid_bit( const unsigned int number );

            // (un)set a single bit
            bitmask operator+( const unsigned int number );
            bitmask operator-( const unsigned int number );
            bitmask operator+=( const unsigned int number );
            bitmask operator-=( const unsigned int number );

            // Combine two masks
            bitmask operator&( const bitmask other );
            bitmask operator/( const bitmask other );
            bitmask operator|( const bitmask other );
            bitmask operator|=( const bitmask other );
            bitmask operator/=( const bitmask other );

            // Negate a mask
            bitmask operator~() const;

            // Check if a bit is set
            bool operator[]( const unsigned int number );

            // Check if two masks share any bits
            bool operator&&( const bitmask other );

            // Check if two masks are equal
            bool operator==( const bitmask other );

            // Check if two masks are not equal
            bool operator!=( const bitmask other );
            bool operator==( const unsigned int other );
            bool operator!=( const unsigned int other );

            bitmask clear();

            // All (un)set masks
            static bitmask all();
            static bitmask none();

            const unsigned int get_value() const;

            const char* toString( const char set, const char unset ) const;

            private:
            value_type value;
        };

        std::ostream& operator<<( std::ostream& stream, const bitmask& mask );
    } // namespace core
} // namespace rnjin