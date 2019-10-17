/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "ecs.hpp"
#include "vulkan_api.hpp"
#include "vulkan_resources.hpp"

namespace rnjin::graphics::vulkan
{
    class renderer : public ecs::system<read_from<internal_resources>>
    {};
} // namespace rnjin::graphics::vulkan