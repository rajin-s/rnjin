/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "resource.hpp"
#include "shader.hpp"
#include "event.hpp"

namespace rnjin::graphics
{
    class material : public resource
    {
        public: // methods
        material();
        material( const string& name );
        material( const string& name, const shader& vertex_shader, const shader& fragment_shader );
        ~material();

        public: // accessors
        let& get_name get_value( name );
        let& get_vertex_shader get_value( vertex_shader );
        let& get_fragment_shader get_value( fragment_shader );
        let inline get_version get_value(version);

        protected: // inherited
        virtual void write_data( io::file& file );
        virtual void read_data( io::file& file );

        protected: // members
        string name;

        // TODO: separate shaders into references
        shader vertex_shader;
        shader fragment_shader;

        version_id version;
    };
} // namespace rnjin::graphics