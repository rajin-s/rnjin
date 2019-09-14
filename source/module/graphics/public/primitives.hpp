/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "mesh.hpp"

namespace rnjin::graphics
{
    namespace primitives
    {
        mesh cube( const float radius );
        mesh sphere( const float radius );
    } // namespace primitive
} // namespace rnjin::graphics