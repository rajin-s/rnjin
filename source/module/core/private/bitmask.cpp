/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "bitmask.hpp"

namespace rnjin
{
    namespace core
    {
        // Is a given bit number valid in a bitmask?
        bool bitmask::is_valid_bit( const uint number )
        {
            return number < ( 8 * sizeof( value_type ) );
        }

        // (un)set a single bit
        bitmask bitmask::operator+( const uint number ) const
        {
            return bitmask( value | bit( number ) );
        }
        bitmask bitmask::operator-( const uint number ) const
        {
            return bitmask( value & ~bit( number ) );
        }
        bitmask bitmask::operator+=( const uint number )
        {
            value = ( value | bit( number ) );
            return *this;
        }
        bitmask bitmask::operator-=( const uint number )
        {
            value = ( value & ~bit( number ) );
            return *this;
        }

        // Combine two masks
        bitmask bitmask::operator&( const bitmask other ) const
        {
            return bitmask( value & other.value );
        }
        bitmask bitmask::operator/( const bitmask other ) const
        {
            return bitmask( value & ~other.value );
        }
        bitmask bitmask::operator|( const bitmask other ) const
        {
            return bitmask( value | other.value );
        }
        bitmask bitmask::operator|=( const bitmask other )
        {
            value = ( value | other.value );
            return *this;
        }
        bitmask bitmask::operator/=( const bitmask other )
        {
            value = ( value & ~other.value );
            return *this;
        }

        // Negate a mask
        bitmask bitmask::operator~() const
        {
            return bitmask( ~value );
        }

        // Check if a bit is set
        bool bitmask::operator[]( const uint number ) const
        {
            return ( value & bit( number ) ) != 0;
        }

        // Check if a bitmask contains a bitmask
        // If this mask contains all the bits in the other value, then & will return the same thing as the other value
        const bool bitmask::contains( const uint other_value ) const
        {
            return ( value & other_value ) == other_value;
        }
        const bool bitmask::contains( const bitmask other ) const
        {
            return ( value & other.value ) == other.value;
        }

        // Check if two masks share any bits
        bool bitmask::operator&&( const bitmask other ) const
        {
            return ( value & other.value ) != 0;
        }

        // Check if two masks are equal
        bool bitmask::operator==( const bitmask other ) const
        {
            return value == other.value;
        }

        // Check if two masks are not equal
        bool bitmask::operator!=( const bitmask other ) const
        {
            return value != other.value;
        }

        bool bitmask::operator==( const uint other ) const
        {
            return value == other;
        }

        bool bitmask::operator!=( const uint other ) const
        {
            return value != other;
        }

        bitmask bitmask::clear()
        {
            value = 0;
            return *this;
        }

        // All (un)set masks
        bitmask bitmask::all()
        {
            return bitmask( ~( 0u ) );
        }
        bitmask bitmask::none()
        {
            return bitmask( 0u );
        }

        // TODO: fix memory leak
        const char* bitmask::toString( const char set, const char unset ) const
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

        std::ostream& operator<<( std::ostream& stream, const bitmask& mask )
        {
            stream << "<" << mask.toString( '-', '_' ) << ">(" << mask.raw_value() << ")";
            return stream;
        }
    } // namespace core
} // namespace rnjin