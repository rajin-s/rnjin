/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "containers.hpp"

namespace rnjin
{
    namespace debug
    {
        const string get_call_string(const char* file_path, const char* function_name, const char* line_number);
    }
} // namespace rnjin