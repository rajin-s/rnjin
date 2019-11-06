/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "resource.hpp"

namespace rnjin
{
    namespace core
    {
        class text_resource : public resource
        {
            public: // methods
            text_resource();
            ~text_resource();

            public: // members
            string content;

            protected: // inherited
            virtual void write_data( io::file& file ) const override;
            virtual void read_data( io::file& file ) override;
        };
    }
}