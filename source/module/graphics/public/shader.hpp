/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "resources/module.h"

namespace rnjin
{
    namespace graphics
    {
        class shader : public resource
        {
            public:
            typedef uint32_t spirv_char;

            public: // enums
            enum class type
            {
                vertex   = 'v',
                fragment = 'f'
            };

            public: // methods
            shader();
            shader( const string& name, const type shader_type );

            ~shader();

            text_resource& get_glsl_resource();
            void set_glsl( const string& new_glsl );
            void compile();

            public: // accessors
            let get_type get_value( shader_type );
            let& get_name get_value( name );
            let& get_spirv get_value( spirv );

            let has_glsl get_value( not glsl.content.empty() );
            let has_spirv get_value( not spirv.empty() );

            protected: // inherited
            virtual void write_data( io::file& file ) const override;
            virtual void read_data( io::file& file ) override;

            private: // members
            string name;
            type shader_type;

            text_resource glsl;
            list<spirv_char> spirv;
        };
    } // namespace graphics
} // namespace rnjin