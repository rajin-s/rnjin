/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "transformations.hpp"

#include <cmath>

#include "math_utilities.hpp"
#include "constants.hpp"

namespace rnjin::core::math
{

    /* -------------------------------------------------------------------------- */
    /*                           Affine Transformations                           */
    /* -------------------------------------------------------------------------- */

    transform::transform()
      : matrix( float4x4::identity() ),         //
        inverse_matrix( float4x4::identity() ), //
        translation_amount(),                   //
        rotation_degrees(),                     //
        rotation_cos( float3( 1, 1, 1 ) ),      //
        rotation_sin(),                         //
        scale_amount( float3( 1, 1, 1 ) )
    {}
    transform::~transform() {}

    void transform::set_position( float3 new_position )
    {
        translation_amount = new_position;

        matrix.rows[0].w = translation_amount.x;
        matrix.rows[1].w = translation_amount.y;
        matrix.rows[2].w = translation_amount.z;
    }
    void transform::set_rotation( float3 new_euler_angles )
    {
        rotation_degrees = new_euler_angles;

        rotation_sin.x = sin( rotation_degrees.x * pi / 180.0 );
        rotation_sin.y = sin( rotation_degrees.y * pi / 180.0 );
        rotation_sin.z = sin( rotation_degrees.z * pi / 180.0 );

        rotation_cos.x = cos( rotation_degrees.x * pi / 180.0 );
        rotation_cos.y = cos( rotation_degrees.y * pi / 180.0 );
        rotation_cos.z = cos( rotation_degrees.z * pi / 180.0 );

        // Apply rotation angles in Y X Z order

        matrix.rows[0].x = scale_amount.x * ( rotation_cos.y * rotation_cos.z + rotation_sin.y * rotation_sin.x * rotation_sin.z );
        matrix.rows[0].y = scale_amount.y * ( rotation_cos.z * rotation_sin.y * rotation_sin.x - rotation_cos.y * rotation_sin.z );
        matrix.rows[0].z = scale_amount.z * ( rotation_cos.x * rotation_sin.y );

        matrix.rows[1].x = scale_amount.x * ( rotation_cos.x * rotation_sin.z );
        matrix.rows[1].y = scale_amount.y * ( rotation_cos.x * rotation_cos.z );
        matrix.rows[1].z = scale_amount.z * ( -rotation_sin.x );

        matrix.rows[2].x = scale_amount.x * ( rotation_cos.y * rotation_sin.x * rotation_sin.z - rotation_cos.z * rotation_sin.y );
        matrix.rows[2].y = scale_amount.y * ( rotation_cos.y * rotation_cos.z * rotation_sin.x + rotation_sin.y * rotation_sin.z );
        matrix.rows[2].z = scale_amount.z * ( rotation_cos.y * rotation_cos.x );
    }
    void transform::set_scale( float3 new_scale )
    {
        scale_amount = new_scale;

        // Apply rotation angles in Y X Z order

        matrix.rows[0].x = scale_amount.x * ( rotation_cos.y * rotation_cos.z + rotation_sin.y * rotation_sin.x * rotation_sin.z );
        matrix.rows[0].y = scale_amount.y * ( rotation_cos.z * rotation_sin.y * rotation_sin.x - rotation_cos.y * rotation_sin.z );
        matrix.rows[0].z = scale_amount.z * ( rotation_cos.x * rotation_sin.y );

        matrix.rows[1].x = scale_amount.x * ( rotation_cos.x * rotation_sin.z );
        matrix.rows[1].y = scale_amount.y * ( rotation_cos.x * rotation_cos.z );
        matrix.rows[1].z = scale_amount.z * ( -rotation_sin.x );

        matrix.rows[2].x = scale_amount.x * ( rotation_cos.y * rotation_sin.x * rotation_sin.z - rotation_cos.z * rotation_sin.y );
        matrix.rows[2].y = scale_amount.y * ( rotation_cos.y * rotation_cos.z * rotation_sin.x + rotation_sin.y * rotation_sin.z );
        matrix.rows[2].z = scale_amount.z * ( rotation_cos.y * rotation_cos.x );
    }

    void transform::translate( float3 offset )
    {
        set_position( translation_amount + offset );
    }
    void transform::scale( float3 amount )
    {
        set_scale( scale_amount + amount );
    }
    void transform::rotate( float3 amount )
    {
        set_rotation( rotation_degrees + amount );
    }

    void transform::set_uniform_scale( float new_scale )
    {
        set_scale( float3( new_scale, new_scale, new_scale ) );
    }
    void transform::uniform_scale( float amount )
    {
        set_scale( scale_amount * amount );
    }


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

        let tan_half_fov = tan( vertical_field_of_view * pi / 180.0 * 0.5 );
        matrix.rows[0].x = 1.0 / ( aspect_ratio * tan_half_fov );
        matrix.rows[1].y = 1.0 / ( tan_half_fov );
    }
    void perspective_projection::set_near_plane_distance( float new_near_plane_distance )
    {
        this->near_plane_distance = new_near_plane_distance;

        let plane_separation = far_plane_distance - near_plane_distance;
        matrix.rows[2].z     = -( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[2].w     = -( 2.0 * near_plane_distance * far_plane_distance ) / plane_separation;
    }
    void perspective_projection::set_far_plane_distance( float new_far_plane_distance )
    {
        this->far_plane_distance = new_far_plane_distance;

        let plane_separation = far_plane_distance - near_plane_distance;
        matrix.rows[2].z     = -( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[2].w     = -( 2.0 * near_plane_distance * far_plane_distance ) / plane_separation;
    }
    void perspective_projection::set_aspect_ratio( float new_ratio )
    {
        this->aspect_ratio = new_ratio;

        let tan_half_fov = tan( vertical_field_of_view * pi / 180.0 * 0.5 );
        matrix.rows[0].x = 1.0 / ( aspect_ratio * tan_half_fov );
    }

    void perspective_projection::update_matrix()
    {
        let tan_half_fov     = tan( vertical_field_of_view * pi / 180.0 * 0.5 );
        let plane_separation = far_plane_distance - near_plane_distance;

        matrix.rows[0].x = 1.0 / ( aspect_ratio * tan_half_fov );
        matrix.rows[1].y = 1.0 / ( tan_half_fov );
        matrix.rows[2].z = -( far_plane_distance + near_plane_distance ) / plane_separation;
        matrix.rows[2].w = -( 2.0 * near_plane_distance * far_plane_distance ) / plane_separation;
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