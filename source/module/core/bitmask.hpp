/*  Part of rnjin
    (c) Rajin Shankar, 2019
        rajinshankar.com
*/

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

#define bit( n ) ( 1 << n )
#define notbit( n ) ( ~( 1 << n ) )

namespace rnjin::core
{
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
        }
        bitmask operator-=( const unsigned int number )
        {
            value = ( value & notbit( number ) );
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
        }
        bitmask operator/=( const bitmask other )
        {
            value = ( value & ~other.value );
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

        // All (un)set masks
        static bitmask all()
        {
            return bitmask( ~0 );
        }
        static bitmask none()
        {
            return bitmask( 0 );
        }

        private:
        value_type value;
    };
} // namespace rnjin::core