/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

// STL data structures
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Aliases for STL types
namespace rnjin
{
    using string = std::string;

    template <typename T>
    using list = std::vector<T>;

    template <typename K, typename V>
    using dictionary = std::unordered_map<K, V>;

    template <typename T>
    using set = std::unordered_set<T>;
} // namespace rnjin