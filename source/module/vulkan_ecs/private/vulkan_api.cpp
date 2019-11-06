/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "vulkan_api.hpp"

#include "vulkan_api_internal.hpp"
#include third_party_library( "glfw3.h" )
#include third_party_library( "vulkan/vulkan.hpp" )

namespace rnjin::graphics::vulkan
{
    log::source::masked vulkan_log         = get_graphics_log().mask( log_flag::vulkan );
    log::source::masked vulkan_log_verbose = get_graphics_log().mask( log_flag::vulkan, log_flag::verbose );
    log::source::masked vulkan_log_errors  = get_graphics_log().mask( log_flag::vulkan, log_flag::errors );

    /* *** *** *** *
     * Public API  *
     * *** *** *** */

    const string& api::get_name()
    {
        static const string name = "Vulkan";
        return name;
    }

    api::api( const string application_name ) : application_name( application_name ), validation( this ), extension( this )
    {
        internal = new api_internal();
    }
    api::~api()
    {
        internal->clean_up();
        delete internal;
    }

    void api::initialize()
    {
        internal->initialize( application_name );
    }

    // Validation Management
    api::validation_management::validation_management( const api* parent ) : parent( parent ) {}
    void api::validation_management::enable( const messages message_type )
    {
        // To do: allow validation enable/disable after instance initialization
        let validation_already_initialized = parent->internal->validation.initialized;
        check_error_condition( return, vulkan_log_errors, validation_already_initialized == true, "Can't enable validation once it's been initialized" );

        if ( (uint) message_type & (uint) messages::errors )
        {
            parent->internal->validation.enable_errors = true;
        }
        if ( (uint) message_type & (uint) messages::warnings )
        {
            parent->internal->validation.enable_warnings = true;
        }
        if ( (uint) message_type & (uint) messages::verbose )
        {
            parent->internal->validation.enable_verbose = true;
        }
    }
    void api::validation_management::disable( const messages message_type )
    {
        // To do: allow validation enable/disable after instance initialization
        let validation_already_initialized = parent->internal->validation.initialized;
        check_error_condition( return, vulkan_log_errors, validation_already_initialized == true, "Can't disable validation once it's been initialized" );

        if ( (uint) message_type & (uint) messages::errors )
        {
            parent->internal->validation.enable_errors = false;
        }
        if ( (uint) message_type & (uint) messages::warnings )
        {
            parent->internal->validation.enable_warnings = false;
        }
        if ( (uint) message_type & (uint) messages::verbose )
        {
            parent->internal->validation.enable_verbose = false;
        }
    }

    // Extension Management
    api::extension_management::extension_management( const api* parent ) : parent( parent ) {}
    void api::extension_management::enable()
    {
        const bool not_implemented = true;
        check_error_condition( return, vulkan_log_errors, not_implemented == true, "Function not implemented" );
    }
    void api::extension_management::disable()
    {
        const bool not_implemented = true;
        check_error_condition( return, vulkan_log_errors, not_implemented == true, "Function not implemented" );
    }
} // namespace rnjin::graphics::vulkan