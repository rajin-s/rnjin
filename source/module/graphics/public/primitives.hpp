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
        mesh quad( const float radius );
        mesh cube( const float radius );
        mesh sphere( const float radius );
    } // namespace primitives
} // namespace rnjin::graphics