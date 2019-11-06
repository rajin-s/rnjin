/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "graphics/public/render_api.hpp"
#include "graphics/public/renderer.hpp"

namespace rnjin::graphics::vulkan
{
    extern log::source::masked vulkan_log;
    extern log::source::masked vulkan_log_verbose;
    extern log::source::masked vulkan_log_errors;

    class api_internal;
    class renderer_internal;

    class api : public render_api
    {
        public:
        api( const string application_name );
        ~api();

        void initialize();

        enum class messages
        {
            verbose  = bit( 0 ),
            warnings = bit( 1 ),
            errors   = bit( 2 ),
            all      = bits( 0, 1, 2 )
        };

        class validation_management
        {
            friend class api;

            public:
            void enable( const messages message_type );
            void disable( const messages message_type );

            private:
            validation_management( const api* parent );
            const api* parent;
        } validation;

        class extension_management
        {
            friend class api;

            public:
            void enable();
            void disable();

            private:
            extension_management( const api* parent );
            const api* parent;
        } extension;

        private:
        string application_name;

        public:
        virtual const string& get_name();

        private:
        api_internal* internal;

        friend class renderer_internal;
        friend class api_internal;
        friend class renderer;
        friend class device;
    };
} // namespace rnjin::graphics::vulkan