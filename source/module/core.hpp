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

// std data structures
#include <string>
#include <unordered_map>
#include <vector>

// include build configuration
#include "core/platform.hpp"

// core data structures
#include "core/macro.hpp"
#include "core/bitmask.hpp"

#define string std::string
#define s(value) std::to_string(value)
#define list std::vector
#define dictionary std::unordered_map

using namespace rnjin;
using namespace rnjin::core;