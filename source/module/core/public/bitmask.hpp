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
            static bool is_valid_bit( const unsigned int number )
            {
                return number < ( 8 * sizeof( value_type ) );
            }

            // (un)set a single bit
            bitmask operator+( const unsigned int number )
            {
                return bitmask( value | bit( number ) );
            }
            bitmask operator-( const unsigned int number )
            {
                return bitmask( value & notbit( number ) );
            }
            bitmask operator+=( const unsigned int number )
            {
                value = ( value | bit( number ) );
                return *this;
            }
            bitmask operator-=( const unsigned int number )
            {
                value = ( value & notbit( number ) );
                return *this;
            }

            // Combine two masks
            bitmask operator&( const bitmask other )
            {
                return bitmask( value & other.value );
            }
            bitmask operator/( const bitmask other )
            {
                return bitmask( value & ~other.value );
            }
            bitmask operator|( const bitmask other )
            {
                return bitmask( value | other.value );
            }
            bitmask operator|=( const bitmask other )
            {
                value = ( value | other.value );
                return *this;
            }
            bitmask operator/=( const bitmask other )
            {
                value = ( value & ~other.value );
                return *this;
            }

            // Negate a mask
            bitmask operator~() const
            {
                return bitmask( ~value );
            }

            // Check if a bit is set
            bool operator[]( const unsigned int number )
            {
                return ( value & bit( number ) ) != 0;
            }

            // Check if two masks share any bits
            bool operator&&( const bitmask other )
            {
                return ( value & other.value ) != 0;
            }

            // Check if two masks are equal
            bool operator==( const bitmask other )
            {
                return value == other.value;
            }

            // Check if two masks are not equal
            bool operator!=( const bitmask other )
            {
                return value != other.value;
            }

            bool operator==( const unsigned int other )
            {
                return value == other;
            }

            bool operator!=( const unsigned int other )
            {
                return value != other;
            }

            bitmask clear()
            {
                value = 0;
                return *this;
            }

            // All (un)set masks
            static bitmask all()
            {
                return bitmask( ~( 0u ) );
            }
            static bitmask none()
            {
                return bitmask( 0u );
            }

            const unsigned int get_value() const
            {
                return value;
            }

            const char* toString(const char set, const char unset) const
            {
                const size_t n = sizeof( value_type ) * 8;
                char* result   = new char[n + 1];

                result[n] = '\0';

                for ( int i = 0; i < n; i++ )
                {
                    if ( value & bit( i ) )
                    {
                        result[i] = set;
                    }
                    else
                    {
                        result[i] = unset;
                    }
                }

                return result;
            }

            private:
            value_type value;
        };

        std::ostream& operator<<( std::ostream& stream, const bitmask& mask )
        {
            stream << "<" << mask.toString('-', '_') << ">(" << mask.get_value() << ")";
            return stream;
        }
    } // namespace core
} // namespace rnjin