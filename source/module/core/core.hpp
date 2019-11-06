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
#include <rnjin.hpp>

// macros
#include "public/macro.hpp"

// Basic container type templates
#include "public/containers.hpp"

// Default modules
#include rnjin_module( math )

// build configuration
#include "public/platform.hpp"

// core data structures
#include "public/bitmask.hpp"
#include "public/event.hpp"
#include "public/unique_id.hpp"

// debugging utilities
#include "public/debug.hpp"

// always used namespaces
// using namespace rnjin;
using namespace rnjin::core;
