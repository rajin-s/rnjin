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

// STL data structures
#include <string>
#include <unordered_map>
#include <vector>

// build configuration
#include "core/platform.hpp"

// core data structures
#include "core/math.hpp"
#include "core/bitmask.hpp"

// macros
#include "core/macro.hpp"

// always used namespaces
using namespace rnjin;
using namespace rnjin::core;