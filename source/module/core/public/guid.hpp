/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

namespace rnjin::core
{
    template <typename T>
    struct guid
    {
        private: // types
        using numeric_type = uint;

        public: // methods
        guid() : value( guid::next_id )
        {
            guid::next_id += 1;
        }
        ~guid() {}

        public: // members
        const numeric_type value;

        public: // static members
        static const numeric_type invalid = 0;

        private: // static members
        static numeric_type next_id{ 1 };

        friend struct std::hash<guid>;
    };
} // namespace rnjin::core

namespace std
{
    template <typename T>
    struct hash<rnjin::core::guid<T>>
    {
        size_t operator()( const rnjin::core::guid<T>& k ) const
        {
            return hash<guid<T>::numeric_type>();
        }
    };
} // namespace std
