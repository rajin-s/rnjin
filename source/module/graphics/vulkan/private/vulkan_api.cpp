/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include <glfw3.h>
#include <vulkan/vulkan.hpp>

#include "vulkan_api.hpp"
#include "vulkan_api_internal.hpp"

namespace rnjin
{
    namespace graphics
    {
        log::source::masked vulkan_log         = get_graphics_log().mask( log_flag::vulkan );
        log::source::masked vulkan_log_verbose = get_graphics_log().mask( log_flag::vulkan, log_flag::verbose );
        log::source::masked vulkan_log_errors  = get_graphics_log().mask( log_flag::vulkan, log_flag::errors );

        /* *** *** *** *
         * Public API  *
         * *** *** *** */

        const string& vulkan::get_name()
        {
            static const string name = "Vulkan";
            return name;
        }

        vulkan::vulkan( const string application_name ) : application_name( application_name ), validation( this ), extension( this )
        {
            _internal = new internal();
        }
        vulkan::~vulkan()
        {
            _internal->clean_up();
            delete _internal;
        }

        void vulkan::initialize()
        {
            _internal->initialize( application_name );
        }

        // Validation Management
        vulkan::validation_management::validation_management( const vulkan* parent ) : parent( parent ) {}
        void vulkan::validation_management::enable( const messages message_type )
        {
            // To do: allow validation enable/disable after instance initialization
            let validation_already_initialized = parent->_internal->validation.initialized;
            check_error_condition( return, vulkan_log_errors, validation_already_initialized == true, "Can't enable validation once it's been initialized" );

            if ( (uint) message_type & (uint) messages::errors )
            {
                parent->_internal->validation.enable_errors = true;
            }
            if ( (uint) message_type & (uint) messages::warnings )
            {
                parent->_internal->validation.enable_warnings = true;
            }
            if ( (uint) message_type & (uint) messages::verbose )
            {
                parent->_internal->validation.enable_verbose = true;
            }
        }
        void vulkan::validation_management::disable( const messages message_type )
        {
            // To do: allow validation enable/disable after instance initialization
            let validation_already_initialized = parent->_internal->validation.initialized;
            check_error_condition( return, vulkan_log_errors, validation_already_initialized == true, "Can't disable validation once it's been initialized" );

            if ( (uint) message_type & (uint) messages::errors )
            {
                parent->_internal->validation.enable_errors = false;
            }
            if ( (uint) message_type & (uint) messages::warnings )
            {
                parent->_internal->validation.enable_warnings = false;
            }
            if ( (uint) message_type & (uint) messages::verbose )
            {
                parent->_internal->validation.enable_verbose = false;
            }
        }

        // Extension Management
        vulkan::extension_management::extension_management( const vulkan* parent ) : parent( parent ) {}
        void vulkan::extension_management::enable()
        {
            const bool not_implemented = true;
            check_error_condition( return, vulkan_log_errors, not_implemented == true, "Function not implemented" );
        }
        void vulkan::extension_management::disable()
        {
            const bool not_implemented = true;
            check_error_condition( return, vulkan_log_errors, not_implemented == true, "Function not implemented" );
        }
    } // namespace graphics
} // namespace rnjin