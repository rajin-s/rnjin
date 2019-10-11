/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "log.hpp"

#include "entity.hpp"
#include "component.hpp"
#include "system.hpp"

namespace rnjin::ecs
{
    // Graphics logs flags used in masked log source creation
    enum class log_flag : uint
    {
        verbose   = 2,
        errors    = 3,
        entity    = 4,
        component = 5,
        system    = 6,
    };

    extern log::source& get_ecs_log();
    extern log::source::masked ecs_log_vernbose;
    extern log::source::masked ecs_log_errors;
} // namespace rnjin::ecs