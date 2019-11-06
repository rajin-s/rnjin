/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "conf.h"
#include "macro.hpp"

#if defined( _WIN32 )
#    define platform_specific( if_windows, if_apple, if_linux, fallback ) if_windows
#elif defined( __APPLE__ )
#    define platform_specific( if_windows, if_apple, if_linux, fallback ) if_apple
#elif defined( __linux__ )
#    define platform_specific( if_windows, if_apple, if_linux, fallback ) if_linux
#else
#    define platform_specific( if_windows, if_apple, if_linux, fallback ) fallback
#endif

namespace rnjin
{
    namespace platform
    {
        enum class os
        {
            unknown = 0,
            windows = 1,
            linux   = 2,
            mac     = 3,

            // android = 4,
            // iOS     = 5,

            // nintendo_switch = 6,
            // xbox_one        = 7,
            // playstation_4   = 8,
        };

        const os osys = os::windows;

        constexpr os operating_system      = platform_specific( os::windows, os::mac, os::linux, os::unknown );
        constexpr char directory_separator = platform_specific( '\\', '/', '/', '/' );

        enum class build_type
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