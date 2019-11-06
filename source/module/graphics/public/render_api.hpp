/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "common.hpp"
#include "renderer.hpp"

namespace rnjin
{
    namespace graphics
    {
        class render_api
        {
            public:
            render_api();
            ~render_api();

            virtual const string& get_name() is_abstract;
        };
    } // namespace graphics
} // namespace rnjin