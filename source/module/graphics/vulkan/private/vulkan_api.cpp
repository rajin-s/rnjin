/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_api.hpp"
#include "vulkan_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            instance::instance( const window<GLFW>& target_window, const bool use_validation_layers )
            {
                _internal = new vulkan::internal();
                _internal->initialize( target_window, use_validation_layers );
            }

            instance::~instance()
            {
                _internal->clean_up();
                delete _internal;
            }
        } // namespace vulkan
    }     // namespace graphics
} // namespace rnjin