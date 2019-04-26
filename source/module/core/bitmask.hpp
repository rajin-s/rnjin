#define bit( n ) ( 1 << n )
#define notbit( n ) ( ~( 1 << n ) )

namespace rnjin::core
{
    struct bitmask
    {
        public:
        bitmask() : value( ~0 ) {}
        bitmask( unsigned int value ) : value( value ) {}

        static bool is_valid_bit( const unsigned int number )
        {
            return number < ( 8 * sizeof( unsigned int ) );
        }

        bitmask operator+( const unsigned int number )
        {
            return bitmask( value | bit( number ) );
        }
        bitmask operator-( const unsigned int number )
        {
            return bitmask( value & notbit( number ) );
        }
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
        bitmask operator~() const
        {
            return bitmask( ~value );
        }

        bitmask operator+=( const unsigned int number )
        {
            value = ( value | bit( number ) );
        }
        bitmask operator-=( const unsigned int number )
        {
            value = ( value & notbit( number ) );
        }
        bitmask operator|=( const bitmask other )
        {
            value = ( value | other.value );
        }
        bitmask operator/=( const bitmask other )
        {
            value = ( value & ~other.value );
        }

        bool operator[]( const unsigned int number )
        {
            return ( value & bit( number ) ) != 0;
        }
        bool operator&&( const bitmask other )
        {
            return ( value & other.value ) != 0;
        }
        bool operator==( const bitmask other )
        {
            return value == other.value;
        }
        bool operator!=( const bitmask other )
        {
            return value != other.value;
        }

        static bitmask all()
        {
            return bitmask( ~0 );
        }
        static bitmask none()
        {
            return bitmask( 0 );
        }

        private:
        unsigned int value;
    };
} // namespace rnjin::core