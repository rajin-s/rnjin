/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

namespace rnjin::core
{
    // Max of an arbitrary number of comparable items
    template <typename T, typename... Ts>
    inline T max_of( T first, T second, Ts... rest )
    {
        if ( first >= second ) { return tmax( first, rest... ); }
        else
        {
            return tmax( second, rest... );
        }
    }

    template <typename T>
    inline T max_of( T single )
    {
        return single;
    }

    // Min of an arbitrary number of comparable items
    template <typename T, typename... Ts>
    inline T min_of( T first, T second, Ts... rest )
    {
        if ( first <= second ) { return tmin( first, rest... ); }
        else
        {
            return tmin( second, rest... );
        }
    }

    template <typename T>
    inline T min_of( T single )
    {
        return single;
    }

    template <typename T>
    inline T clamp( T value, T min_value, T max_value )
    {
        if ( value <= min_value ) { return min_value; }
        else if ( value >= max_value )
        {
            return max_value;
        }
        else
        {
            return value;
        }
    }
} // namespace rnjin::core