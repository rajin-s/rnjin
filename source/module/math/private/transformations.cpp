/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "transformations.hpp"

#include <cmath>

#include "math_utilities.hpp"

namespace rnjin::core::math
{
    /* -------------------------------------------------------------------------- */
    /*                           Perspective Projection                           */
    /* -------------------------------------------------------------------------- */

    perspective_projection::perspective_projection()
      : aspect_ratio( 1 ),            //
        vertical_field_of_view( 75 ), //
        near_plane_distance( 0.1 ),   //
        far_plane_distance( 100 ),    //
        matrix()                      //
    {
        update_matrix();
    }
    perspective_projection::perspective_projection( float vertical_field_of_view, float near_plane_distance, float far_plane_distance )
      : pass_member( vertical_field_of_view ), //
        pass_member( near_plane_distance ),    //
        pass_member( far_plane_distance ),     //
        matrix()                               //
    {
        update_matrix();
    }
    perspective_projection::perspective_projection( float aspect_ratio, float vertical_field_of_view, float near_plane_distance, float far_plane_distance )
      : pass_member( aspect_ratio ),           //
        pass_member( vertical_field_of_view ), //
        pass_member( near_plane_distance ),    //
        pass_member( far_plane_distance ),     //
        matrix()                               //
    {
        update_matrix();
    }
    perspective_projection::~perspective_projection() {}

    void perspective_projection::set_field_of_view( float new_angle )
    {
        this->vertical_field_of_view = new_angle;

        let tan_half_fov = tan( vertical_field_of_view * 0.5 );
        matrix.rows[0].x = 1.0 / ( aspect_ratio * tan_half_fov );
        matrix.rows[1].y = 1.0 / ( tan_half_fov );
    }
    void perspective_projection::set_near_plane_distance( float new_near_plane_distance )
    {
        this->near_plane_distance = new_near_plane_distance;

        let plane_separation = far_plane_distance - near_plane_distance;
        matrix.rows[2].z     = ( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[2].w     = ( 2.0 * near_plane_distance * far_plane_distance ) / plane_separation;
    }
    void perspective_projection::set_far_plane_distance( float new_far_plane_distance )
    {
        this->far_plane_distance = new_far_plane_distance;

        let plane_separation = far_plane_distance - near_plane_distance;
        matrix.rows[2].z     = ( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[2].w     = ( 2.0 * near_plane_distance * far_plane_distance ) / plane_separation;
    }
    void perspective_projection::set_aspect_ratio( float new_ratio )
    {
        this->aspect_ratio = new_ratio;

        let tan_half_fov = tan( vertical_field_of_view * 0.5 );
        matrix.rows[0].x = 1.0 / ( aspect_ratio * tan_half_fov );
    }

    void perspective_projection::update_matrix()
    {
        let tan_half_fov     = tan( vertical_field_of_view * 0.5 );
        let plane_separation = far_plane_distance - near_plane_distance;

        matrix.rows[0].x = 1.0 / ( aspect_ratio * tan_half_fov );
        matrix.rows[1].y = 1.0 / ( tan_half_fov );
        matrix.rows[2].z = ( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[2].w = ( 2.0 * near_plane_distance * far_plane_distance ) / plane_separation;
        matrix.rows[3].z = -1.0;
    }


    /* -------------------------------------------------------------------------- */
    /*                           Orthographic Projection                          */
    /* -------------------------------------------------------------------------- */

    orthographic_projection::orthographic_projection()
      : aspect_ratio( 1.0 ),          //
        world_size( 1.0, 1.0 ),       //
        near_plane_distance( -10.0 ), //
        far_plane_distance( 100.0 ),  //
        matrix()
    {
        update_matrix();
    }
    orthographic_projection::orthographic_projection( float height )
      : aspect_ratio( 1.0 ),                         //
        world_size( height * aspect_ratio, height ), //
        near_plane_distance( -10.0 ),                //
        far_plane_distance( 100.0 ),                 //
        matrix()
    {
        update_matrix();
    }
    orthographic_projection::orthographic_projection( float height, float near_plane_distance, float far_plane_distance )
      : aspect_ratio( 1.0 ),                         //
        world_size( height * aspect_ratio, height ), //
        pass_member( near_plane_distance ),          //
        pass_member( far_plane_distance ),           //
        matrix()
    {
        update_matrix();
    }
    orthographic_projection::orthographic_projection( float aspect_ratio, float height, float near_plane_distance, float far_plane_distance )
      : aspect_ratio( aspect_ratio ),                //
        world_size( height * aspect_ratio, height ), //
        pass_member( near_plane_distance ),          //
        pass_member( far_plane_distance ),           //
        matrix()
    {
        update_matrix();
    }

    orthographic_projection::~orthographic_projection() {}

    void orthographic_projection::set_height( float new_height )
    {
        this->world_size.x = new_height * aspect_ratio;
        this->world_size.y = new_height;

        matrix.rows[0].x = 1.0 / world_size.x;
        matrix.rows[1].y = -1.0 / world_size.y;
    }
    void orthographic_projection::set_width( float new_width )
    {
        this->world_size.x = new_width;
        this->world_size.y = new_width / aspect_ratio;

        matrix.rows[0].x = 1.0 / world_size.x;
        matrix.rows[1].y = -1.0 / world_size.y;
    }
    void orthographic_projection::set_near_plane_distance( float new_near_plane_distance )
    {
        this->near_plane_distance = new_near_plane_distance;

        let plane_separation = far_plane_distance - near_plane_distance;
        matrix.rows[2].z     = 1.0 / plane_separation;
        matrix.rows[2].w     = -( far_plane_distance + near_plane_distance ) / plane_separation;
    }
    void orthographic_projection::set_far_plane_distance( float new_far_plane_distance )
    {
        this->far_plane_distance = new_far_plane_distance;

        let plane_separation = far_plane_distance - near_plane_distance;
        matrix.rows[2].z     = 1.0 / plane_separation;
        matrix.rows[2].w     = -( far_plane_distance + near_plane_distance ) / plane_separation;
    }
    void orthographic_projection::set_aspect_ratio( float new_ratio )
    {
        this->aspect_ratio = new_ratio;
        this->world_size.x = this->world_size.y * aspect_ratio;

        matrix.rows[0].x = 1.0 / world_size.x;
        matrix.rows[1].y = -1.0 / world_size.y;
    }

    void orthographic_projection::update_matrix()
    {
        let plane_separation = far_plane_distance - near_plane_distance;

        matrix.rows[0].x = 1.0 / world_size.x;
        matrix.rows[1].y = -1.0 / world_size.y;
        matrix.rows[2].z = 1.0 / plane_separation;
        matrix.rows[2].w = -( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[3].w = 1.0;
    }
} // namespace rnjin::core::math