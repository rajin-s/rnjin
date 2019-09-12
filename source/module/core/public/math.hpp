/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "containers.hpp"
#include <ostream>

namespace rnjin
{
    namespace core
    {
        // Forward declarations

        template <typename T>
        struct vector2;

        template <typename T>
        struct vector3;

        template <typename T>
        struct vector4;


        // Two-dimensional container of a given type that supports arithmetic operations
        template <typename T>
        struct vector2
        {
            vector2() : x(), y() {}
            vector2( T x, T y ) : x( x ), y( y ) {}
            T x, y;

            inline const vector2<T> operator+( const vector2<T> other ) const
            {
                return vector2<T>( x + other.x, y + other.y );
            }
            inline const vector2<T> operator-( const vector2<T> other ) const
            {
                return vector2<T>( x - other.x, y - other.y );
            }
            inline const vector2<T> operator*( const T scalar ) const
            {
                return vector2<T>( x * scalar, y * scalar );
            }

            inline const bool operator!=( const vector2<T> other ) const
            {
                return x != other.x || y != other.y;
            }
            inline const bool operator==( const vector2<T> other ) const
            {
                return !operator!=( other );
            }

            inline operator vector3<T>() const
            {
                return vector3<T>( x, y, 0 );
            }
        };

        template<typename T>
        inline const vector2<T> operator*( const T scalar, const vector2<T> vector )
        {
            return vector * scalar;
        }

        // Three-dimensional container of a given type that supports arithmetic operations
        template <typename T>
        struct vector3
        {
            vector3() : x(), y(), z() {}
            vector3( T x, T y, T z ) : x( x ), y( y ), z( z ) {}
            T x, y, z;

            inline const vector3<T> operator+( const vector3<T> other ) const
            {
                return vector3<T>( x + other.x, y + other.y, z + other.z );
            }
            inline const vector3<T> operator-( const vector3<T> other ) const
            {
                return vector3<T>( x - other.x, y - other.y, z - other.z );
            }
            inline const vector3<T> operator*( const T scalar ) const
            {
                return vector3<T>( x * scalar, y * scalar, z * scalar );
            }

            inline const bool operator!=( const vector3<T> other ) const
            {
                return x != other.x || y != other.y || z != other.z;
            }
            inline const bool operator==( const vector3<T> other ) const
            {
                return !operator!=( other );
            }

        };

        template<typename T>
        inline const vector3<T> operator*( const T scalar, const vector3<T> vector )
        {
            return vector * scalar;
        }

        // Four-dimensional container of a given type that supports arithmetic operations
        template <typename T>
        struct vector4
        {
            vector4() : x(), y(), z(), w() {}
            vector4( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {}
            T x, y, z, w;

            inline const vector4<T> operator+( const vector4<T> other ) const
            {
                return vector4<T>( x + other.x, y + other.y, z + other.z, w + other.w );
            }
            inline const vector4<T> operator-( const vector4<T> other ) const
            {
                return vector4<T>( x - other.x, y - other.y, z - other.z, w - other.w );
            }
            inline const vector4<T> operator*( const T scalar ) const
            {
                return vector4<T>( x * scalar, y * scalar, z * scalar, w * scalar );
            }

            inline const bool operator!=( const vector4<T> other ) const
            {
                return x != other.x || y != other.y || z != other.z || w != other.w;
            }
            inline const bool operator==( const vector4<T> other ) const
            {
                return !operator!=( other );
            }

        };

        template<typename T>
        inline const vector4<T> operator*( const T scalar, const vector4<T> vector )
        {
            return vector * scalar;
        }

        template <typename T>
        std::ostream& operator<<( std::ostream& stream, vector2<T> v )
        {
            stream << "(" << v.x << ", " << v.y << ")";
            return stream;
        }
        template <typename T>
        std::ostream& operator<<( std::ostream& stream, vector3<T> v )
        {
            stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
            return stream;
        }
        template <typename T>
        std::ostream& operator<<( std::ostream& stream, vector4<T> v )
        {
            stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
            return stream;
        }

        using float2 = vector2<float>;
        using float3 = vector3<float>;
        using float4 = vector4<float>;

        using double2 = vector2<double>;
        using double3 = vector3<double>;
        using double4 = vector4<double>;

        using int2 = vector2<int>;
        using int3 = vector3<int>;
        using int4 = vector4<int>;

        using uint2 = vector2<uint>;
        using uint3 = vector3<uint>;
        using uint4 = vector4<uint>;

        // Template math functions

        // Max of an arbitrary number of comparable items
        template <typename T, typename... Ts>
        inline T tmax( T first, T second, Ts... rest )
        {
            if ( first >= second )
            {
                return tmax( first, rest... );
            }
            else
            {
                return tmax( second, rest... );
            }
        }

        template <typename T>
        inline T tmax( T single )
        {
            return single;
        }

        // Min of an arbitrary number of comparable items
        template <typename T, typename... Ts>
        inline T tmin( T first, T second, Ts... rest )
        {
            if ( first <= second )
            {
                return tmin( first, rest... );
            }
            else
            {
                return tmin( second, rest... );
            }
        }

        template <typename T>
        inline T tmin( T single )
        {
            return single;
        }

        template <typename T>
        inline T clamp( T value, T min_value, T max_value )
        {
            if ( value <= min_value )
            {
                return min_value;
            }
            else if ( value >= max_value )
            {
                return max_value;
            }
            else
            {
                return value;
            }
        }

    } // namespace core
} // namespace rnjin