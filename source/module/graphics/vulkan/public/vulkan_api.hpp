/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "render_api.hpp"
#include "renderer.hpp"

namespace rnjin
{
    namespace graphics
    {
        extern log::source::masked vulkan_log;
        extern log::source::masked vulkan_log_verbose;
        extern log::source::masked vulkan_log_errors;

        class vulkan : public render_api
        {
            public:
            vulkan( const string application_name );
            ~vulkan();

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
                friend class vulkan;

                public:
                void enable( const messages message_type );
                void disable( const messages message_type );

                private:
                validation_management( const vulkan* parent );
                const vulkan* parent;
            } validation;

            class extension_management
            {
                friend class vulkan;

                public:
                void enable();
                void disable();

                private:
                extension_management( const vulkan* parent );
                const vulkan* parent;
            } extension;

            private:
            string application_name;

            public:
            virtual const string& get_name();

            // Internal structure that actually does API calls
            // note: separated to prevent general engine code from needing to include / being able to access the actual Vulkan library (PIMPL)
            private:
            class internal;
            internal* _internal;

            friend class renderer<vulkan>;
        };
    } // namespace graphics
} // namespace rnjin