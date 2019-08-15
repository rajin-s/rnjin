// /* *** ** *** ** *** ** *** *
//  * Part of rnjin            *
//  * (c) Rajin Shankar, 2019  *
//  *        rajinshankar.com  *
//  * *** ** *** ** *** ** *** */

// #include "render_api.hpp"
// #include "renderer.hpp"


// namespace rnjin
// {
//     namespace graphics
//     {
//         renderer::renderer( render_api& target_api ) : api( target_api )
//         {
//             api.add_client( *this );
//         }

//         renderer::~renderer()
//         {
//             api.remove_client( *this );
//         }
//     } // namespace graphics
// } // namespace rnjin