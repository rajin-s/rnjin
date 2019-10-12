/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

/*  Core engine functionality include with
        - STL containers and symbolic type names (potentially replaced later with custom ones)
        - General purpose engine data structures
        - General purpose namespaces
 */

#pragma once

// Basic container type templates
#include "containers.hpp"

// build configuration
#include "platform.hpp"

// core data structures
#include "bitmask.hpp"
#include "math.hpp"
#include "event.hpp"
#include "unique_id.hpp"

// macros
#include "macro.hpp"

// debugging utilities
#include "debug.hpp"

// always used namespaces
// using namespace rnjin;
using namespace rnjin::core;