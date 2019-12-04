/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "log/module.h"

namespace rnjin::graphics
{
    // Graphics logs flags used in masked log source creation
    enum class log_flag : uint
    {
        verbose = 2,
        errors  = 3,
        vulkan  = 4
    };

    // Graphics output log
    extern log::source& get_graphics_log();
    extern log::source::masked graphics_log_verbose;
    extern log::source::masked graphics_log_errors;
} // namespace rnjin::graphics
