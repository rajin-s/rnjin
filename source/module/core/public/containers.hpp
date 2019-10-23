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

    // named numeric types
    using usize = std::size_t;

    // numeric wrappers
    class version_id
    {
        public:
        inline version_id() : number( version_id::first_number ) {}
        inline ~version_id() {}

        inline bool update_to( const version_id other )
        {
            bool result = number != other.number;
            number      = other.number;
            return result;
        }

        inline version_id& operator++(int _)
        {
            number++;
            return *this;
        }

        inline bool is_first()
        {
            return number == version_id::first_number;
        }
        inline bool is_invalid()
        {
            return number == version_id::invalid_number;
        }

        inline static version_id invalid()
        {
            return version_id( version_id::invalid_number );
        }

        private:
        using numeric_type = uint64;
        numeric_type number;

        inline version_id( numeric_type number ) : number( number ) {}

        static constexpr numeric_type first_number   = 0;
        static constexpr numeric_type invalid_number = ~0;
    };

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
        inline range( const uint first, const uint limit ) : start_i( first ), end_i( limit ) {}
        inline range( const uint limit ) : start_i( 0 ), end_i( limit ) {}

        inline range& begin()
        {
            return *this;
        }
        inline range& end()
        {
            return *this;
        }
        inline const uint operator*()
        {
            return start_i;
        }
        inline const bool operator!=( const range& other )
        {
            return start_i < end_i;
        }
        inline range& operator++()
        {
            start_i++;
            return *this;
        }

        private:
        uint start_i;
        const uint end_i;
    };

    // A Rust-style iterator object that can be used in a for loop
    // ie. for(T& : iterator<T&>(T*, T*))
    template <typename T>
    class mutable_iterator
    {
        private: // types
        using list_type          = list<T>;
        using list_iterator_type = typename list<T>::iterator;

        public: // methods
        inline mutable_iterator( list_type& source ) : start_iterator( source.begin() ), end_iterator( source.end() ) {}
        inline mutable_iterator& begin()
        {
            return *this;
        }
        inline mutable_iterator& end()
        {
            return *this;
        }
        inline T& operator*()
        {
            return *start_iterator;
        }
        inline const bool operator!=( const mutable_iterator& other )
        {
            return start_iterator != end_iterator;
        }
        inline mutable_iterator& operator++()
        {
            start_iterator++;
            return *this;
        }

        inline bool is_valid()
        {
            return start_iterator != end_iterator;
        }
        inline void advance()
        {
            start_iterator++;
        }

        private:
        list_iterator_type start_iterator;
        const list_iterator_type end_iterator;
    };
    template <typename T>
    class const_iterator
    {
        private: // types
        using list_type          = list<T>;
        using list_iterator_type = typename list<T>::iterator;

        public: // methods
        inline const_iterator( list_type& source ) : start_iterator( source.begin() ), end_iterator( source.end() ) {}
        inline const_iterator& begin()
        {
            return *this;
        }
        inline const_iterator& end()
        {
            return *this;
        }
        inline const T& operator*()
        {
            return *start_iterator;
        }
        inline const bool operator!=( const const_iterator& other )
        {
            return start_iterator != end_iterator;
        }
        inline const_iterator& operator++()
        {
            start_iterator++;
            return *this;
        }

        inline bool is_valid()
        {
            return start_iterator != end_iterator;
        }
        inline void advance()
        {
            start_iterator++;
        }

        private:
        list_iterator_type start_iterator;
        const list_iterator_type end_iterator;
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