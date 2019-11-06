/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include standard_library( ostream )

namespace rnjin::core
{
    // A simple template struct for definite class-level globally unique ID numbers
    template <typename T>
    struct unique_id
    {
        public: // types
        using value_type = uint;

        public: // methods
        // Default constructor increments global counter
        unique_id() : id( unique_id::next_id )
        {
            unique_id::next_id += 1;
            // std::cout << "\n\nMAKE GUID (" << unique_id::next_id << ")\n\n";
        }
        // Copy constructor doesn't increase global counter
        unique_id( const unique_id& original ) : id( original.id ) {}
        ~unique_id() {}

        // Assignment
        // unique_id& operator=( const unique_id& other )
        // {
        //     id = other.id;
        //     return *this;
        // }

        // Compare two IDs
        inline bool operator==( const unique_id& second ) const
        {
            return id == second.id;
        }
        inline bool operator!=( const unique_id& second ) const
        {
            return id != second.id;
        }
        inline bool operator<( const unique_id& second ) const
        {
            return id < second.id;
        }
        inline bool operator>( const unique_id& second ) const
        {
            return id > second.id;
        }
        inline bool operator<=( const unique_id& second ) const
        {
            return id <= second.id;
        }
        inline bool operator>=( const unique_id& second ) const
        {
            return id >= second.id;
        }

        public: // accessors
        inline let value get_value( id );
        inline let is_valid get_value( id != 0 );

        private: // members
        value_type id;

        private: // methods
        // Used to construct an invalid ID
        unique_id( value_type id ) : id( id ) {}

        public: // static methods
        const unique_id invalid()
        {
            return unique_id( 0 );
        }

        private: // static members
        // Start IDs at 1, so 0 will always be invalid
        static value_type next_id;

        public:
        friend struct std::hash<unique_id>;
    };

    template <typename T>
    typename unique_id<T>::value_type unique_id<T>::next_id{ 1 };

} // namespace rnjin::core


template <typename T>
struct std::hash<rnjin::core::unique_id<T>>
{
    inline size_t operator()( const rnjin::core::unique_id<T>& id ) const
    {
        return std::hash<unique_id<T>::value_type>()( id.id );
    }
};

template <typename T>
std::ostream& operator<<( std::ostream& stream, const rnjin::core::unique_id<T>& id )
{
    stream << "id=" << id.value();
    return stream;
}
