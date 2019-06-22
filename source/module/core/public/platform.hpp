/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "conf.h"

namespace rnjin
{
    namespace platform
    {
        enum os
        {
            windows,
            linux,
            mac,
            android,
            iOS,
            nintendo_switch,
            xbox,
            playstation,
        };
        constexpr os operating_system =
#ifdef __linux__
        os::linux;
#elif __APPLE__
        os::mac;
#else
        os::windows;
#endif

        enum build_type
        {
            release        = 0,
            test           = 1,
            debug          = 2,
            debug_internal = 3
        };
        constexpr build_type build = build_type::BUILD_MODE;
    } // namespace platform
} // namespace rnjin


#define min_debug( mode ) if ( platform::build >= platform::build_type::##mode )
#define require_os( name ) if ( platform::operating_system == platform::os::##name )