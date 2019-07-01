/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

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
            vector2( T x, T y ) : x( x ), y( y ) {}
            T x, y;

            const vector2<T> operator+( const vector2<T> other )
            {
                return vector2<T>( x + other.x, y + other.y );
            }
            const vector2<T> operator-( const vector2<T> other )
            {
                return vector2<T>( x - other.x, y - other.y );
            }
            const vector2<T> operator*( const T scalar )
            {
                return vector2<T>( x * scalar, y * scalar );
            }

            operator vector3<T>() const
            {
                return vector3<T>( x, y, 0 );
            }
        };

        // Three-dimensional container of a given type that supports arithmetic operations
        template <typename T>
        struct vector3
        {
            vector3( T x, T y, T z ) : x( x ), y( y ), z( z ) {}
            T x, y, z;

            const vector3<T> operator+( const vector3<T> other )
            {
                return vector3<T>( x + other.x, y + other.y, z + other.z );
            }
            const vector3<T> operator-( const vector3<T> other )
            {
                return vector3<T>( x - other.x, y - other.y, z - other.z );
            }
            const vector3<T> operator*( const T scalar )
            {
                return vector3<T>( x * scalar, y * scalar, z * scalar );
            }
        };

        // Four-dimensional container of a given type that supports arithmetic operations
        template <typename T>
        struct vector4
        {
            vector4( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {}
            T x, y, z, w;

            const vector4<T> operator+( const vector4<T> other )
            {
                return vector4<T>( x + other.x, y + other.y, z + other.z, w + other.w );
            }
            const vector4<T> operator-( const vector4<T> other )
            {
                return vector4<T>( x - other.x, y - other.y, z - other.z, w - other.w );
            }
            const vector4<T> operator*( const T scalar )
            {
                return vector4<T>( x * scalar, y * scalar, z * scalar, w * scalar );
            }
        };

        template <typename T>
        std::ostream& operator<<( std::ostream& stream, vector2<T> v )
        {
            stream << "(" << v.x << ", " << v.y << ")";
        }
        template <typename T>
        std::ostream& operator<<( std::ostream& stream, vector3<T> v )
        {
            stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        }
        template <typename T>
        std::ostream& operator<<( std::ostream& stream, vector4<T> v )
        {
            stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
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

        // Template math functions
        
        // Max of an arbitrary number of comparable items
        template <typename T, typename... Ts>
        T max( T first, T second, Ts... rest )
        {
            if (first >= second)
            {
                return max(first, rest...);
            }
            else
            {
                return max(second, rest...);
            }
        }

        template <typename T>
        T max( T single )
        {
            return single;
        }
        
        // Min of an arbitrary number of comparable items
        template <typename T, typename... Ts>
        T min( T first, T second, Ts... rest )
        {
            if (first <= second)
            {
                return min(first, rest...);
            }
            else
            {
                return min(second, rest...);
            }
        }

        template <typename T>
        T min( T single )
        {
            return single;
        }

        template <typename T>
        T clamp(T value, T min, T max)
        {
            if (value <= min)
            {
                return min;
            }
            else if (value >= max)
            {
                return max;
            }
            else
            {
                return value;
            }
        }

    } // namespace core
} // namespace rnjin