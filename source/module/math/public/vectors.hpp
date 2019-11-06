/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include standard_library( ostream )

#include "core/public/containers.hpp"
// #include <ostream>

namespace rnjin::core
{

/* -------------------------------------------------------------------------- */
/*                            Template Vector Types                           */
/* -------------------------------------------------------------------------- */
#pragma region template_vectors

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

        inline const vector2 operator+( const vector2 other ) const
        {
            return vector2( x + other.x, y + other.y );
        }
        inline const vector2 operator-( const vector2 other ) const
        {
            return vector2( x - other.x, y - other.y );
        }
        inline const vector2 operator*( const T scalar ) const
        {
            return vector2( x * scalar, y * scalar );
        }

        // Dot (Scalar) product
        inline const T operator*( const vector2 other ) const
        {
            return x * other.x + y * other.y;
        }

        inline const bool operator!=( const vector2 other ) const
        {
            return x != other.x || y != other.y;
        }
        inline const bool operator==( const vector2 other ) const
        {
            return !operator!=( other );
        }

        inline operator vector3<T>() const
        {
            return vector3<T>( x, y, 0 );
        }
    };

    template <typename T>
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

        inline const vector3 operator+( const vector3 other ) const
        {
            return vector3( x + other.x, y + other.y, z + other.z );
        }
        inline const vector3 operator-( const vector3 other ) const
        {
            return vector3( x - other.x, y - other.y, z - other.z );
        }
        inline const vector3 operator*( const T scalar ) const
        {
            return vector3( x * scalar, y * scalar, z * scalar );
        }

        // Dot (Scalar) Product
        inline const T operator*( const vector3 other ) const
        {
            return x * other.x + y * other.y + z * other.z;
        }

        // Cross (Vector) Product
        inline const vector3 operator^( const vector3 other ) const
        {
            return vector3(
                y * other.z - z * other.y, //
                z * other.x - x * other.z, //
                x * other.y - y * other.x  //
            );
        }

        inline const bool operator!=( const vector3 other ) const
        {
            return x != other.x || y != other.y || z != other.z;
        }
        inline const bool operator==( const vector3 other ) const
        {
            return !operator!=( other );
        }
    };

    template <typename T>
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

        inline const vector4 operator+( const vector4 other ) const
        {
            return vector4( x + other.x, y + other.y, z + other.z, w + other.w );
        }
        inline const vector4 operator-( const vector4 other ) const
        {
            return vector4( x - other.x, y - other.y, z - other.z, w - other.w );
        }
        inline const vector4 operator*( const T scalar ) const
        {
            return vector4( x * scalar, y * scalar, z * scalar, w * scalar );
        }

        // Dot (Scalar) Product
        inline const T operator*( const vector4 other ) const
        {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        inline const bool operator!=( const vector4 other ) const
        {
            return x != other.x || y != other.y || z != other.z || w != other.w;
        }
        inline const bool operator==( const vector4 other ) const
        {
            return !operator!=( other );
        }
    };

    template <typename T>
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

#pragma endregion template_vectors

/* -------------------------------------------------------------------------- */
/*                                  Matrices                                  */
/* -------------------------------------------------------------------------- */
#pragma region matrices

    struct float4x4
    {
        inline float4x4( float4 row_0, float4 row_1, float4 row_2, float4 row_3 ) : rows{ row_0, row_1, row_2, row_3 } {}
        // Default constructor produces an identity matrix
        inline float4x4()
          : rows{
                float4( 1, 0, 0, 0 ), //
                float4( 0, 1, 0, 0 ), //
                float4( 0, 0, 1, 0 ), //
                float4( 0, 0, 0, 1 )  //
            }
        {}
        float4 rows[4];

        inline float4 operator*( const float4 other )
        {
            return float4(
                rows[0] * other, //
                rows[1] * other, //
                rows[2] * other, //
                rows[3] * other  //
            );
        }
    };

#pragma endregion
} // namespace rnjin::core