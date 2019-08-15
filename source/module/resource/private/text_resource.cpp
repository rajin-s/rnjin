/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "text_resource.hpp"

namespace rnjin
{
    namespace core
    {
        text_resource::text_resource() {}
        text_resource::~text_resource() {}

        void text_resource::write_data( io::file& file )
        {
            file.write_all_text( content );
        }
        void text_resource::read_data( io::file& file )
        {
            content = file.read_all_text();
        }
    } // namespace core
} // namespace rnjin