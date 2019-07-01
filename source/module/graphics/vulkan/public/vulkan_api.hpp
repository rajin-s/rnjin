/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include <glfw.hpp>

namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            class internal;
            class instance
            {
                public:
                instance( const window<GLFW>& target_window, const bool use_validation_layers );
                ~instance();

                private:
                vulkan::internal* _internal;
            };
        } // namespace vulkan
    }     // namespace graphics
} // namespace rnjin
