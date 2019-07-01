/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        namespace vulkan
        {
            const int log_channel_vulkan = 2;

            void internal::initialize( const window<GLFW>& target_window, const bool enable_validation )
            {
                graphics_log.print( "Starting Vulkan initialization...", log_channel_vulkan );

                if ( enable_validation )
                {
                    _validation.enable();
                }

                _instance.initialize( _validation );
                _validation.initialize( _instance );

                _surface.initialize( _instance, target_window );

                _device.initialize( _instance, _surface, _validation );

                _surface.create_swap_chain( _device );

                graphics_log.print( "Finished Vulkan initialization", log_channel_vulkan );
            }

            void internal::clean_up()
            {
                graphics_log.print( "Starting Vulkan cleanup...", log_channel_vulkan );

                _surface.clean_up( _instance, _device );
                _device.clean_up();
                _validation.clean_up( _instance );
                _instance.clean_up();

                graphics_log.print( "Finished Vulkan cleanup", log_channel_vulkan );
            }
        } // namespace vulkan

    } // namespace graphics
} // namespace rnjin