/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "ecs.hpp"

namespace rnjin::ecs
{
    log::source& get_ecs_log()
    {
        static log::source ecs_log(
            "rnjin.ecs", log::output_mode::immediately, log::output_mode::immediately,
            {
                { "verbose", (uint) log_flag::verbose, false },    //
                { "errors", (uint) log_flag::errors, true },       //
                { "entity", (uint) log_flag::entity, true },       //
                { "component", (uint) log_flag::component, true }, //
                { "system", (uint) log_flag::system, true },       //
            }                                                      //
        );
        return ecs_log;
    }

    log::source::masked ecs_log_verbose = get_ecs_log().mask( log_flag::verbose );
    log::source::masked ecs_log_errors  = get_ecs_log().mask( log_flag::errors );
} // namespace rnjin::ecs