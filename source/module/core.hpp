#pragma once

// std data structures
#include <string>
#include <unordered_map>
#include <vector>

// core data structures
#include <core/macro.hpp>
#include <core/bitmask.hpp>

#define string std::string
#define str(value) std::to_string(value)
#define list std::vector
#define dictionary std::unordered_map

using namespace rnjin::core;