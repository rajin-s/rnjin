/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "common.hpp"

namespace rnjin
{
    namespace graphics
    {
        log::source& get_graphics_log()
        {
            static log::source graphics_log(
                "rnjin.graphics", log::output_mode::immediately, log::output_mode::immediately,
                {
                    { "verbose", (uint) log_flag::verbose, false }, //
                    { "errors", (uint) log_flag::errors, true },    //
                    { "vulkan", (uint) log_flag::vulkan, false }    //
                }                                                   //
            );
            return graphics_log;
        }

        log::source::masked graphics_log_verbose = get_graphics_log().mask( log_flag::verbose );
        log::source::masked graphics_log_errors  = get_graphics_log().mask( log_flag::errors );

    } // namespace graphics
} // namespace rnjin